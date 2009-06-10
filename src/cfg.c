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

#include "../config.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfg.h"
#include "default_settings.h"
#include "lists.h"
#include "util.h"

struct setting {
        char *key;
        char *value;
};

static void add_mod(char *mod);
static int find_mod(struct cfg *cur, char *mod);
static void add_key(char *mod, char *key, char *value);
static int find_key(struct setting *cur, char *key);
static FILE *get_cfg_file(void);
static void clear_settings(struct setting *cur_s);

struct list *cfg_ls;

#define IS_TRUE(c) ( ((c) == 'y') || ((c) == 'Y') || \
                     ((c) == 't') || ((c) == 'T') || \
                     ((c) == '1') )

#define IS_FALSE(c) ( ((c) == 'n') || ((c) == 'N') || \
                      ((c) == 'f') || ((c) == 'F') || \
                      ((c) == '0') )

#define MAX_LINE_SIZE 1024

/** 
 * @brief Add a mod to the configuration list
 * 
 * @param mod The name of the mod to add
 */
static void add_mod(char *mod)
{
        struct cfg *new_mod;

        new_mod = malloc(sizeof(struct cfg));
        new_mod->mod = dstrdup(mod);
        new_mod->setting_ls = init_list();

        add_node(cfg_ls, new_mod);
}

/** 
 * @brief Callback used in finding a mod in the coming functions.
 * 
 * @param cur cfg node whose mod we check against
 * @param mod Mod name we're looking for
 * 
 * @return 1 if this is the node we want, 0 if not
 */
static int find_mod(struct cfg *cur, char *mod)
{
        if (!dstrcasecmp(cur->mod, mod))
                return 1;

        return 0;
}

/** 
 * @brief Add a key (setting, option) and its value to its respective mod
 * 
 * @param mod Name of the mod to add the key to
 * @param key Name of key
 * @param value Value of the key to add (as a string)
 */
static void add_key(char *mod, char *key, char *value)
{
        struct cfg *cur;
        struct setting *new_setting;

        cur = find_node(cfg_ls, &find_mod, mod);
        if (cur == NULL)
                return;

        new_setting = malloc(sizeof(struct setting));
        new_setting->key = dstrdup(key);
        new_setting->value = dstrdup(value);

        add_node(cur->setting_ls, new_setting);
}

/** 
 * @brief Callback used in finding a key in the coming functions.
 * 
 * @param cur setting node whose key we check against
 * @param mod Key name we're looking for
 * 
 * @return 1 if this is the node we want, 0 if not
 */
static int find_key(struct setting *cur, char *key)
{
        if (!dstrcasecmp(cur->key, key))
                return 1;

        return 0;
}

/** 
 * @brief Get the char value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * @param otherwise Value to return if we cannot determine a key
 * 
 * @return Malloc'd char value of either key or 'otherwise'
 */
const char *get_char_key(char *mod, char *key, char *otherwise)
{
        struct cfg *cur;
        struct setting *cur_s;

        cur = find_node(cfg_ls, &find_mod, mod);
        if (cur == NULL) {
                if (otherwise != NULL)
                        return otherwise;

                return NULL;
        }

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if ((cur_s != NULL) && (cur_s->value != NULL))
                return cur_s->value;
        else if (otherwise != NULL)
                return otherwise;

        return NULL;
}

/** 
 * @brief Get the int value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * @param otherwise Value to return if we cannot determine a key
 * 
 * @return The int value of key, or 'otherwise'
 */
int get_int_key(char *mod, char *key, int otherwise)
{
        struct cfg *cur;
        struct setting *cur_s;

        cur = find_node(cfg_ls, &find_mod, mod);
        if (cur == NULL)
                return otherwise;

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if ((cur_s != NULL) && (cur_s->value != NULL))
                return atoi(cur_s->value);

        return otherwise;
}

/** 
 * @brief Get the double value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * @param otherwise Value to return if we cannot determine a key
 * 
 * @return The double value of key, or 'otherwise'
 */
double get_double_key(char *mod, char *key, double otherwise)
{
        struct cfg *cur;
        struct setting *cur_s;

        cur = find_node(cfg_ls, &find_mod, mod);
        if (cur == NULL)
                return otherwise;

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if ((cur_s != NULL) && (cur_s->value != NULL))
                return atof(cur_s->value);

        return otherwise;
}

/** 
 * @brief Get the boolean value of a key
 * 
 * @param mod Name of mod
 * @param key Name of key
 * @param otherwise Value to return if we cannot determine a key
 * 
 * @return The boolean value of key, or 'otherwise'
 */
int get_bool_key(char *mod, char *key, int otherwise)
{
        struct cfg *cur;
        struct setting *cur_s;

        cur = find_node(cfg_ls, &find_mod, mod);
        if (cur == NULL)
                return otherwise;

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if ((cur_s != NULL) && (cur_s->value != NULL)) {
                if (IS_TRUE(cur_s->value[0]))
                        return 1;
                else if (IS_FALSE(cur_s->value[0]))
                        return 0;
        }

        return otherwise;
}

/** 
 * @brief Parses the main donky configuration file
 */
void parse_cfg(void)
{
        FILE *cfg_file;
        char str[MAX_LINE_SIZE];
        char mod[64];
        char key[64];
        char value[128];
        const char *format[5];  /* sscanf formats */
        int mod_check;          /* bool */

        cfg_file = get_cfg_file();
        cfg_ls = init_list();      /* initialize our cfg list */

        mod_check = 0;  /* set this to 1 when we get our first mod */

        format[0] = " [%63[a-zA-Z0-9_-]]";                /* [mod]         */
        format[1] = " %63[a-zA-Z0-9_-] = \"%127[^\"]\" "; /* key = "value" */
        format[2] = " %63[a-zA-Z0-9_-] = '%127[^\']' ";   /* key = 'value' */
        format[3] = " %63[a-zA-Z0-9_-] = %127[^;\n] ";    /* key = value   */
        format[4] = " %63[a-zA-Z0-9_-] ";                 /* key           */

        while ((fgets(str, sizeof(str), cfg_file)) != NULL) {
                if (is_comment(str)) {
#ifdef ENABLE_DEBUGGING
                        chomp(str);
                        printf("Skipping comment [%s]\n", str);
#endif
                        continue;
                }

                if (sscanf(str, format[0], mod) == 1) {
                        add_mod(mod);
                        if (mod_check == 0)
                                mod_check = 1;
                        continue;
                }

                /* if we have no mod, don't parse for keys & values */
                if (mod_check == 0)
                        continue;

                if (sscanf(str, format[1], key, value) == 2) {
                        goto handle_key;
                } else if (sscanf(str, format[2], key, value) == 2) {
                        goto handle_key;
                } else if (sscanf(str, format[3], key, value) == 2) {
                        trim_t(value);
                        goto handle_key;
                } else if (sscanf(str, format[4], key) == 1) {
                        strfcpy(value, "True", sizeof(value));
                        goto handle_key;
                }

                continue;

handle_key:
                /* values of "" or '' are interpreted as False */
                if (!strcmp(value, "\"\"") || !strcmp(value, "''"))
                        strfcpy(value, "False", sizeof(value));

                add_key(mod, key, value);

#ifdef ENABLE_DEBUGGING
                printf("added-> mod [%s] key [%s] value [%s]\n",
                       mod, key, value);
                printf("  char [%s] int [%d] double [%f] bool [%d]\n\n",
                       get_char_key(mod, key, "ERROR!"),
                       get_int_key(mod, key, -1),
                       get_double_key(mod, key, -1),
                       get_bool_key(mod, key, -1));
#endif
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
        char *cfg_path = NULL;
        FILE *cfg_file;

        stracpy(&cfg_path, getenv("HOME"));
        stracat(&cfg_path, "/" DEFAULT_CONF);

        cfg_file = fopen(cfg_path, "r");
        if (cfg_file == NULL) {
                fprintf(stderr,
                        "Warning: ~/%s file not found.\n",
                        DEFAULT_CONF);

                stracpy(&cfg_path, SYSCONFDIR "/" DEFAULT_CONF_GLOBAL);

                cfg_file = fopen(cfg_path, "r");
                if (cfg_file == NULL) {
                        free(cfg_path);
                        fprintf(stderr,
                                "Error: %s/%s file not found.\n",
                                SYSCONFDIR, DEFAULT_CONF_GLOBAL);
                        exit(EXIT_FAILURE);
                }
        }

        free(cfg_path);

        return cfg_file;
}

/** 
 * @brief Frees the contents of the cfg struct passed to it.
 * 
 * @param cur cfg struct to clean
 */
void clear_cfg(struct cfg *cur)
{
        free(cur->mod);
        del_list(cur->setting_ls, &clear_settings);
}

/** 
 * @brief Frees the contents of the setting struct passed to it.
 * 
 * @param cur_s setting struct to clean
 */
static void clear_settings(struct setting *cur_s)
{
        free(cur_s->key);
        free(cur_s->value);
}

