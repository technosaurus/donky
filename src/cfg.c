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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "default_settings.h"
#include "util.h"

#define MAX_LINE_SIZE 1024

struct setting {
        char *key;
        char *value;

        struct setting *next;
};

struct setting_ls {
        struct setting *first;
        struct setting *last;
};

struct cfg {
        char *mod;
        struct setting_ls *setting_ls;

        struct cfg *next;
};

struct cfg_ls {
        struct cfg *first;
        struct cfg *last;
};

static void init_cfg_ls(void);
static void add_mod(const char *mod);
static struct cfg *find_mod(const char *mod);
static void add_key(const char *mod, const char *key, const char *value);
static struct setting *find_key(const char *mod, const char *key);
static FILE *get_cfg_file(void);
static void clear_settings(struct setting_ls *ls);

static struct cfg_ls *cfg_ls;

/** 
 * @brief Return an initialized configuration list
 */
static void init_cfg_ls(void)
{
        extern struct cfg_ls *cfg_ls;

        cfg_ls = malloc(sizeof(struct cfg_ls));
        cfg_ls->first = NULL;
        cfg_ls->last = NULL;
}

/** 
 * @brief Add a mod to the configuration list
 */
static void add_mod(const char *mod)
{
        extern struct cfg_ls *cfg_ls;
        struct cfg *new;

        new = malloc(sizeof(struct cfg));
        new->next = NULL;
        new->mod = dstrdup(mod);
        new->setting_ls = malloc(sizeof(struct setting_ls));
        new->setting_ls->first = NULL;
        new->setting_ls->last = NULL;

        if (cfg_ls->last) {
                cfg_ls->last->next = new;
                cfg_ls->last = new;
        } else {
                cfg_ls->first = new;
                cfg_ls->last = new;
        }
}

/** 
 * @brief Search for and return a mod from the configuration list.
 *        Return NULL if it isn't found.
 */
static struct cfg *find_mod(const char *mod)
{
        extern struct cfg_ls *cfg_ls;
        struct cfg *cur;

        cur = cfg_ls->first;
        while (cur != NULL) {
                if (!dstrcasecmp(cur->mod, mod))
                        return cur;
                cur = cur->next;
        }

        return NULL;
}

/** 
 * @brief Add key and its value to a mod's list of settings
 */
static void add_key(const char *mod, const char *key, const char *value)
{
        struct cfg *cur;
        struct setting *new_set;

        cur = find_mod(mod);
        if (cur == NULL)
                return;

        new_set = malloc(sizeof(struct setting));
        new_set->next = NULL;
        new_set->key = dstrdup(key);
        if (value != NULL)
                new_set->value = dstrdup(value);
        else
                new_set->value = NULL;

        if (cur->setting_ls->last != NULL) {
                cur->setting_ls->last->next = new_set;
                cur->setting_ls->last = new_set;
        } else {
                cur->setting_ls->first = new_set;
                cur->setting_ls->last = new_set;
        }

        DEBUGF(("Added: mod [%s] key [%s] value [%s]\n"
                "-- char [%s] int [%d] double [%f] bool [%d]\n\n",
                mod, key, value,
                get_char_key(mod, key, "(null)"),
                get_int_key(mod, key, -1),
                get_double_key(mod, key, -1),
                get_bool_key(mod, key, -1)));
}

/** 
 * @brief Search for a mod's setting by name and return the setting struct.
 */
static struct setting *find_key(const char *mod, const char *key)
{
        struct cfg *cur_mod;
        struct setting *cur_set;

        cur_mod = find_mod(mod);
        if (cur_mod == NULL)
                return NULL;

        cur_set = cur_mod->setting_ls->first;
        while (cur_set != NULL) {
                if (!dstrcasecmp(cur_set->key, key))
                        return cur_set;
                cur_set = cur_set->next;
        }

        return NULL;
}

/**
 * See "cfg.h" for information on the following 4 functions.
 */

/** 
 * @brief Search a mod for a setting, and return the value of the setting
 *        in char * format. If the setting cannot be found or has no value,
 *        return the 'otherwise' parameter.
 */
const char *get_char_key(const char *mod,
                         const char *key,
                         const char *otherwise)
{
        struct setting *cur;

        cur = find_key(mod, key);
        if ((cur == NULL) || (cur->value == NULL))
                return otherwise;

        return cur->value;
}

/** 
 * @brief Search a mod for a setting, and return the value of the setting
 *        in int format. If the setting cannot be found or has no value,
 *        return the 'otherwise' parameter.
 */
int get_int_key(const char *mod, const char *key, int otherwise)
{
        struct setting *cur;

        cur = find_key(mod, key);
        if ((cur == NULL) || (cur->value == NULL))
                return otherwise;

        return atoi(cur->value);
}

/** 
 * @brief Search a mod for a setting, and return the value of the setting
 *        in double format. If the setting cannot be found or has no value,
 *        return the 'otherwise' parameter.
 */
double get_double_key(const char *mod, const char *key, double otherwise)
{
        struct setting *cur;

        cur = find_key(mod, key);
        if ((cur == NULL) || (cur->value == NULL))
                return otherwise;

        return atof(cur->value);
}

/** 
 * @brief Search a mod for a setting, and return the value of the setting
 *        in boolean (int) format. (1 = true, 0 = false)
 *        If the setting cannot be found or has no value, return the
 *        'otherwise' parameter.
 */
#define IS_TRUE(c) ( ((c) == 'y') || ((c) == 'Y') || \
                     ((c) == 't') || ((c) == 'T') || \
                     ((c) == '1') )

#define IS_FALSE(c) ( ((c) == 'n') || ((c) == 'N') || \
                      ((c) == 'f') || ((c) == 'F') || \
                      ((c) == '0') )

int get_bool_key(const char *mod, const char *key, int otherwise)
{
        struct setting *cur;

        cur = find_key(mod, key);
        if ((cur == NULL) || (cur->value == NULL))
                goto out;

        if (IS_TRUE(cur->value[0]))
                return 1;
        else if (IS_FALSE(cur->value[0]))
                return 0;
out:
        return otherwise;
}

/**
 * @brief Parse the main donky configuration file.
 */
void parse_cfg(void)
{
        FILE *cfg_file;
        char str[MAX_LINE_SIZE];
        char mod[64];
        char key[64];
        char value[128];
        int have_mod;   /* bool - do we have a mod? */

        cfg_file = get_cfg_file();
        init_cfg_ls();  /* initialize the cfg list */
        have_mod = 0;   /* set this to 1 when we get our first mod */

        while ((fgets(str, sizeof(str), cfg_file)) != NULL) {
                if (is_comment(str)) {
                        DEBUGF(("Skipping comment [%s]\n", chomp(str)));
                        continue;
                }

#define         MOD_FMT " [%63[a-zA-Z0-9_-]]"
                if (sscanf(str, MOD_FMT, mod) == 1) {
                        add_mod(mod);
                        if (have_mod == 0)
                                have_mod = 1;
                        continue;
                }

                /* if we have no mod, don't parse for keys & values */
                if (have_mod == 0)
                        continue;

#define         KEY_FMT_1 " %63[a-zA-Z0-9_-] = \"%127[^\"]\" "
#define         KEY_FMT_2 " %63[a-zA-Z0-9_-] = '%127[^\']' "
#define         KEY_FMT_3 " %63[a-zA-Z0-9_-] = %127[^;\n] "
#define         KEY_FMT_4 " %63[a-zA-Z0-9_-] "
                if (sscanf(str, KEY_FMT_1, key, value) == 2) {
                        add_key(mod, key, value);
                } else if (sscanf(str, KEY_FMT_2, key, value) == 2) {
                        add_key(mod, key, value);
                } else if (sscanf(str, KEY_FMT_3, key, value) == 2) {
                        trim_t(value);
                        if (!strcmp(value, "\"\"") || !strcmp(value, "''"))
                                add_key(mod, key, NULL);
                        else
                                add_key(mod, key, value);
                } else if (sscanf(str, KEY_FMT_4, key) == 1) {
                        add_key(mod, key, "True");
                }
        }

        fclose(cfg_file);
}

/**
 * @brief Load configuration. We check for the local configuration, usually
 *        ~/.donkyrc, then if not found, we try to open the system wide
 *        configuration. If none are found, we blow this joint.
 *
 * @return Pointer to an open .donkyrc file, if successful.
 */
static FILE *get_cfg_file(void)
{
        char *path = NULL;
        FILE *file;

        stracpy(&path, getenv("HOME"));
        stracat(&path, "/" DEFAULT_CONF);
        file = fopen(path, "r");
        if (file == NULL) {
                fprintf(stderr, "Warning: Can't open config file %s.\n", path);
                stracpy(&path, SYSCONFDIR "/" DEFAULT_CONF_GLOBAL); 
                file = fopen(path, "r");
                if (file == NULL)
                        goto error;
        }

        free(path);
        return file;

error:
        fprintf(stderr, "Error: Also can't open system config file %s. "
                        "Exiting.\n", path);
        exit(EXIT_FAILURE);
}

/** 
 * @brief Free everything in the configuration list.
 */
void clear_cfg(void)
{
        extern struct cfg_ls *cfg_ls;
        struct cfg *cur;
        struct cfg *next;

        cur = cfg_ls->first;
        while (cur != NULL) {
                next = cur->next;
                free(cur->mod);
                clear_settings(cur->setting_ls);
                free(cur);
                cur = next;
        }

        free(cfg_ls);
        cfg_ls = NULL;
}

/** 
 * @brief Helper function to clear_cfg(). Frees a mod's settings list.
 */
static void clear_settings(struct setting_ls *ls)
{
        struct setting *cur;
        struct setting *next;

        cur = ls->first;
        while (cur != NULL) {
                next = cur->next;
                free(cur->key);
                free(cur->value);
                free(cur);
                cur = next;
        }
        
        free(ls);
}

