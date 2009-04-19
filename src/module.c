/*
 * This file is part of donky.
 *
 * donky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * donky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <dirent.h>
#include <dlfcn.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "module.h"
#include "text.h"
#include "util.h"
#include "lists.h"

/* internal prototypes */
int module_add(char *name, void *handle, void *destroy);
struct module *module_find(char *name);
int module_var_add(char *parent, char *name, char *method, double timeout, enum variable_type type);
struct module_var *module_var_find(char *name);
int module_load(char *path);
void module_unload(char *name);
void *module_get_sym(void *handle, char *name);

/* Globals. */
struct first_last *module_fl = NULL;
struct first_last *module_var_fl = NULL;
int module_var_used = 0;

/**
 * @brief Add a new link to the "module" linked list.
 *
 * @param name Unique name for the module
 * @param handle Pointer to the code memory segment handle
 *
 * @return 1 for success, 0 for failure
 */
int module_add(char *name, void *handle, void *destroy)
{
        struct module *n = malloc(sizeof(struct module));

        /* Fill in the module structure. */
        memset(n->name, 0, sizeof(n->name));
        strncpy(n->name, name, sizeof(n->name) - 1);
        n->handle = handle;
        n->destroy = destroy;

        /* Add to linked list. */
        if (module_fl == NULL)
                module_fl = init_list();

        add_node(module_fl, n);

        return 1;
}

/**
 * @brief Callback for find_node for module_find.
 *
 * @param cur
 * @param match
 *
 * @return 1 = match, 0 = not match
 */
int module_find_cb(struct module *cur, char *match)
{
        if (!strcasecmp(cur->name, match))
                return 1;

        return 0;
}

/**
 * @brief Find a module link.
 *
 * @param name Unique name of module
 *
 * @return The module link matching the given name, or NULL if not found
 */
struct module *module_find(char *name)
{
        return find_node(module_fl, &module_find_cb, name);
}

/**
 * @brief Add a module_var link.
 *
 * @param parent The unique name of this var's parent module
 * @param name A unique var name
 * @param method The name of the method to call
 * @param type Variable type,  @see variable_type
 *
 * @return 0 for utter phail, 1 for success
 */
int module_var_add(char *parent, char *name, char *method, double timeout, enum variable_type type)
{
        struct module *mod = module_find(parent);

        if (mod == NULL)
                return 0;

        /* Check that this variable is used in the [text] section.  If it isn't,
         * we won't even add it to the linked list.  Memorah efficiencah! */
        if (text_section_var_find(name) == NULL) {
                printf("Var [%s] was not used.\n", name);
                return 0;
        }

        module_var_used = 1; /* Trigger that a module var was used from this module. */
        struct module_var *n = malloc(sizeof(struct module_var));

        /* Fill in module_var structure. */
        memset(n->name, 0, sizeof(n->name));
        memset(n->method, 0, sizeof(n->method));
        strncpy(n->name, name, sizeof(n->name) - 1);
        strncpy(n->method, method, sizeof(n->method) - 1);
        n->type = type;
        n->sym = module_get_sym(mod->handle, method);
        n->timeout = 0.0;
        n->last_update = 0.0;
        n->parent = mod;

        /* Set the timeout.  User configured timeouts take precedence over
         * module defined default! */
        double user_timeout = get_double_key("timeout", name, timeout);

        /* Set a pointer to this node in all text sections using this variable. */
        text_section_var_modvar(name, n, user_timeout);

        /* Add to linked list. */
        if (module_var_fl == NULL)
                module_var_fl = init_list();

        add_node(module_var_fl, n);

        return 1;
}

/**
 * @brief Callback for find_node in module_var_find.
 *
 * @param cur
 * @param match
 *
 * @return 1 = match, 0 = not match
 */
int module_var_find_cb(struct module_var *cur, char *match)
{
        if (!strcasecmp(cur->name, match))
                return 1;

        return 0;
}

/**
 * @brief Find a module var link.
 *
 * @param name Unique name of module var
 *
 * @return The module var link matching the given name, or NULL if not found
 */
struct module_var *module_var_find(char *name)
{
        return find_node(module_var_fl, &module_var_find_cb, name);
}

/**
 * @brief Add cron job.
 *
 * @param parent Unique name of parent module
 * @param name Unique name of this cron job
 * @param method Name of method
 * @param timeout Timeout in seconds
 *
 * @return 1 success, 0 fail
 */
int module_var_cron_add(char *parent, char *name, char *method, double timeout)
{
        struct module *mod = module_find(parent);

        if (mod == NULL)
                return 0;
        
        struct module_var *n = malloc(sizeof(struct module_var));

        /* Fill in module_var structure. */
        memset(n->name, 0, sizeof(n->name));
        memset(n->method, 0, sizeof(n->method));
        strncpy(n->name, name, sizeof(n->name) - 1);
        strncpy(n->method, method, sizeof(n->method) - 1);
        n->type = VARIABLE_CRON;
        n->sym = module_get_sym(mod->handle, method);

        /* Set the timeout.  User configured timeouts take precedence over
         * module defined default! */
        double user_timeout = get_double_key("cron", name, timeout);

        printf("Adding [%s] to cron with timeout [%f]\n", name, user_timeout);
        
        n->timeout = user_timeout;
        n->last_update = 0.0;
        n->parent = mod;

        /* Add to linked list. */
        if (module_var_fl == NULL)
                module_var_fl = init_list();

        add_node(module_var_fl, n);

        return 1;
}

/**
 * @brief Call back for del_node in module_var_cron_clear.
 *
 * @param cur
 * @param parent
 *
 * @return 1 = match, 0 = not match
 */
int module_var_cron_clear_cb(struct module_var *cur, struct module *parent)
{
        if (cur->parent == parent)
                return 1;

        return 0;
}

/**
 * @brief Clear current cron jobs of a given module.
 *
 * @param parent Pointer to parent module
 */
void module_var_cron_clear(struct module *parent)
{
        del_nodes(module_var_fl, &module_var_cron_clear_cb, parent, NULL);
}

/**
 * @brief Callback for act_on_list in module_var_cron_exec.
 *
 * @param cur
 */
void module_var_cron_exec_cb(struct module_var *cur)
{
        void *(*sym)(void);

        if (cur->type == VARIABLE_CRON &&
            (get_time() - cur->last_update) >= cur->timeout) {
                if ((sym = cur->sym) != NULL)
                        sym();

                cur->last_update = get_time();
        }
}

/**
 * @brief Run all cron jobs.
 */
void module_var_cron_exec(void)
{
        act_on_list(module_var_fl, &module_var_cron_exec_cb);
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
        void (*module_init)(void);
        void *module_destroy;

        if ((handle = dlopen(path, RTLD_LAZY)) == NULL) {
                fprintf(stderr, "%s: Could not open: %s\n", path, dlerror());
                return 0;
        }

        module_name = module_get_sym(handle, "module_name");
        module_init = module_get_sym(handle, "module_init");
        module_destroy = module_get_sym(handle, "module_destroy");

        /* Check for required symbols. */
        if (!(module_name || module_init || module_destroy)) {
                fprintf(stderr, "%s: Did not have required symbols, skipping!\n");
                return 0;
        }

        module_add(module_name, handle, module_destroy);
        module_var_used = 0;
        module_init();

        /* No module vars were loaded from this module, so lets unload it. */
        if (!module_var_used) {
                printf("Nobody's using module [%s], unloading!\n", module_name);
                module_unload(module_name);
        }
        
        return 1;
}

/**
 * @brief Callback for del_node in module_unload.
 *
 * @param cur
 * @param match
 *
 * @return 1 = match, 0 = not match
 */
int module_unload_find(struct module *cur, char *match)
{
        if (!strcasecmp(cur->name, match))
                return 1;

        return 0;
}

/**
 * @brief Callback for del_node in module_unload.
 *
 * @param cur
 */
void module_unload_free(struct module *cur)
{
        void (*module_destroy)(void);

        if ((module_destroy = cur->destroy) != NULL)
                module_destroy();
                
        dlclose(cur->handle);
        module_var_cron_clear(cur);
}

/**
 * @brief Unload a module.
 *
 * @param name Unique name of the module
 */
void module_unload(char *name)
{
        del_node(module_fl, &module_unload_find, name, &module_unload_free);
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
                
        while (dir = readdir(d)) {
                if (!strcmp(dir->d_name, ".") ||
                    !strcmp(dir->d_name, "..") ||
                    dir->d_type == DT_DIR)
                        continue;

                sptr = strrchr(dir->d_name, '.');
                if (sptr && !strcasecmp(sptr, ".so")) {
                        snprintf(full_path,
                                 sizeof(full_path),
                                 "%s/%s",
                                 LIBDIR, dir->d_name);
                                 
                        printf("Attempting to load: %s\n", full_path);
                        module_load(full_path);
                }
        }

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
        char *error;

        if (error = dlerror()) {
                fprintf(stderr, "module_get_sym: problem finding %s: %s\n", name, error);
                return NULL;
        }

        return location;
}

/**
 * @brief Clear the module and module_var linked lists.
 */
void clear_module(void)
{
        del_list(module_var_fl, NULL);
        module_var_fl = NULL;
        
        del_list(module_fl, &module_unload_free);
        module_fl = NULL;
}
