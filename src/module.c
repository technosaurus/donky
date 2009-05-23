/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "c99.h"
#include "config.h"
#include "module.h"
#include "util.h"
#include "lists.h"

/* Globals. */
struct module *m_start = NULL;
struct module *m_end = NULL;

struct module_var *mv_start = NULL;
struct module_var *mv_end = NULL;

static bool first_load = true;

/* Function prototypes. */
static struct module *module_add(const char *name,
                                 const char *path,
                                 void *handle,
                                 void *destroy);
static struct module *module_find_by_name(const char *name);

/**
 * @brief Add a module_var link.
 *
 * @param parent Parent module
 * @param name Unique name of this var
 * @param method Name of method
 * @param timeout Timeout in seconds
 * @param type Variable type,  @see variable_type
 *
 * @return 1 success, 0 fail
 */
int module_var_add(const struct module *parent,
                   char *name,
                   const char *method,
                   double timeout,
                   enum variable_type type)
{
        double user_timeout;
        struct module_var *find;
        struct module_var *n;

        if (parent == NULL)
                return 0;

        find = module_var_find_by_name(name);
        n = (find) ? find : malloc(sizeof(struct module_var));

        /* Fill in module_var structure. */
        snprintf(n->name, sizeof(n->name), "%s", name);
        snprintf(n->method, sizeof(n->method), "%s", method);
        n->type = type;
        n->sym = NULL;

        if (!find) {
                n->prev = NULL;
                n->next = NULL;
        }

        /* Set the timeout.  User configured timeouts take precedence over
         * module defined default! */
        if (type == VARIABLE_CRON)
                user_timeout = get_double_key("cron", name, timeout);
        else
                user_timeout = get_double_key("timeout", name, timeout);

        /*printf("Adding [%s] with timeout [%f]\n", name, user_timeout);*/
        
        n->timeout = user_timeout;
        n->last_update = 0.0;
        n->sum = 0;
        n->parent = (struct module *) parent;

        if (find)
                return 1;

        /* Add node to linked list. */
        if (mv_end == NULL) {
                mv_start = n;
                mv_end = n;
        } else {
                mv_end->next = n;
                n->prev = mv_end;
                mv_end = n;
        }
        
        return 1;
}

/**
 * @brief Find module var by name.
 *
 * @param name Module var name
 *
 * @return Module var
 */
struct module_var *module_var_find_by_name(const char *name)
{
        struct module_var *cur = mv_start;

        while (cur) {
                if (!strcasecmp(cur->name, name))
                        return cur;
                
                cur = cur->next;
        }

        return NULL;
}

/**
 * @brief Run cron jobs.
 */
void module_var_cron_exec(void)
{
        void *(*sym)(void);
        struct module_var *cur = mv_start;

        while (cur) {
                if (cur->type == VARIABLE_CRON &&
                    (get_time() - cur->last_update) >= cur->timeout) {
                        if ((sym = cur->sym) != NULL) {
                                sym();
                                cur->last_update = get_time();
                        }
                }
                
                cur = cur->next;
        }
}

/**
 * @brief Load up the symbol pointer for crons.
 *
 * @param parent Module parent
 */
void module_var_cron_init(struct module *parent)
{
        struct module_var *cur = mv_start;

        while (cur) {
                if (cur->parent == parent)
                        cur->sym = module_get_sym(parent->handle, cur->method);
                
                cur = cur->next;
        }
}

/**
 * @brief Add module linked list node.
 *
 * @param name Module name
 * @param path Path to module
 * @param handle dlopen handle
 * @param destroy Pointer to destroy method
 *
 * @return Module just added
 */
static struct module *module_add(const char *name,
                                 const char *path,
                                 void *handle,
                                 void *destroy)
{
        struct module *find = module_find_by_name(name);
        struct module *n = (find) ? find : malloc(sizeof(struct module));

        printf("Loaded module: %s\n", name);

        snprintf(n->name, sizeof(n->name), "%s", name);
        n->path = d_strcpy(path);
        n->handle = handle;
        n->destroy = destroy;
        n->clients = 0;

        if (!find) {
                n->prev = NULL;
                n->next = NULL;
        }

        if (find)
                return n;

        /* Add node to linked list. */
        if (m_end == NULL) {
                m_start = n;
                m_end = n;
        } else {
                m_end->next = n;
                n->prev = m_end;
                m_end = n;
        }

        return n;
}

/**
 * @brief Find module by name.
 *
 * @param name Module name
 *
 * @return
 */
static struct module *module_find_by_name(const char *name)
{
        struct module *cur = m_start;

        while (cur) {
                if (!strcasecmp(cur->name, name))
                        return cur;
                
                cur = cur->next;
        }

        return NULL;
}

/**
 * @brief Unload a module.
 *
 * @param name Module name
 */
void module_unload(struct module *cur)
{
        void *(*destroy)(void);
        struct module_var *mv;

        if (!cur)
                return;

        printf("Unloading module %s... ", cur->name);

        destroy = cur->destroy;
        destroy();
        dlclose(cur->handle);

        cur->destroy = NULL;
        cur->handle = NULL;
        cur->clients = 0;

        /* Set all module_var symbol pointers to NULL if this is their
         * parent module. */
        mv = mv_start;

        while (mv) {
                if (mv->parent == cur)
                        mv->sym = NULL;
                
                mv = mv->next;
        }

        printf("done.\n");
}

/**
 * @brief Load a module.
 *
 * @param path Path to the module
 *
 * @return 0 for failure, 1 for success
 */
int module_load(char *path)
{
        void *handle;
        char *module_name;
        void (*module_init)(struct module *);
        void *module_destroy;
        struct module *cur;

        if ((handle = dlopen(path, RTLD_LAZY)) == NULL) {
                fprintf(stderr, "%s: Could not open: %s\n", path, dlerror());
                return 0;
        }

        module_name = module_get_sym(handle, "module_name");
        module_init = module_get_sym(handle, "module_init");
        module_destroy = module_get_sym(handle, "module_destroy");

        /* Check for required symbols. */
        if (!(module_name || module_init || module_destroy)) {
                fprintf(stderr,
                        "%s: Did not have required symbols, skipping!\n",
                        path);
                return 0;
        }

        
        cur = module_add(module_name, path, handle, module_destroy);
        module_init(cur);

        /* When donky is first loaded, we want to immediately unload
         * modules after letting all the module variables register. */
        if (first_load)
                module_unload(cur);
        
        return 1;
}

/**
 * @brief Load all modules in the main lib directory.
 */
void module_load_all(void)
{
        DIR *d;
        struct dirent *dir;
        char full_path[MAXPATHLEN];
        char *sptr;

        if ((d = opendir(LIBDIR)) == NULL) {
                fprintf(stderr,
                        "Modules directory (%s), could not be opened!\n",
                        LIBDIR);
                return;
        }

        /* We want to unload all modules after loading. */
        first_load = true;

        while ((dir = readdir(d))) {
                if (!strcmp(dir->d_name, ".") ||
                    !strcmp(dir->d_name, ".."))
                        continue;

                sptr = strrchr(dir->d_name, '.');
                if (sptr && !strcasecmp(sptr, ".so")) {
                        snprintf(full_path,
                                 sizeof(full_path) - sizeof(char),
                                 "%s/%s",
                                 LIBDIR, dir->d_name);

                        printf("Attempting to load: %s\n", full_path);
                        module_load(full_path);
                }
        }

        /* Modules will stay loaded whenever module_load is called now. */
        first_load = false;
        
        closedir(d);
}

/**
 * @brief Return a symbol from a module.
 *
 * @param handle Module handle
 * @param name Name of symbol
 *
 * @return Address of symbol
 */
void *module_get_sym(void *handle, char *name)
{
        /* Clear any existing errors. */
        dlerror();

        void *location = dlsym(handle, name);
        const char *error;

        if ((error = dlerror())) {
                fprintf(stderr, "module_get_sym: problem finding %s: %s\n", name, error);
                return NULL;
        }

        return location;
}

/**
 * @brief Cleanup all the module linked list crap.
 */
void clear_module(void)
{
        void *(*destroy)(void);
        struct module *m = m_start;
        struct module *mn;
        struct module_var *mv = mv_start;
        struct module_var *mvn;

        while (m) {
                mn = m->next;

                free(m->path);
                if ((destroy = m->destroy))
                        destroy();
                if (m->handle)
                        dlclose(m->handle);
                free(m);
                
                m = mn;
        }

        while (mv) {
                mvn = mv->next;

                free(mv);
                
                mv = mvn;
        }

        m_start = m_end = NULL;
        mv_start = mv_end = NULL;

        first_load = true;
}
