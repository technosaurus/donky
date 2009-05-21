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

#define _GNU_SOURCE
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
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
bool parse(const char *str, const char *format, int n, ...);
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
void add_mod(char *mod)
{
        struct cfg *new_mod;

        new_mod = malloc(sizeof(struct cfg));
        new_mod->mod = d_strcpy(mod);
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
        if (!strcasecmp(cur->mod, mod))
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
void add_key(char *mod, char *key, char *value)
{
        struct cfg *cur;
        struct setting *new_setting;

        cur = find_node(cfg_ls, &find_mod, mod);
        if (!cur)
                return;

        new_setting = malloc(sizeof(struct setting));
        new_setting->key = d_strcpy(key);
        new_setting->value = d_strcpy(value);

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
        if(!strcasecmp(cur->key, key))
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
char *get_char_key(char *mod, char *key, char *otherwise)
{
        struct cfg *cur;
        struct setting *cur_s;

        cur = find_node(cfg_ls, &find_mod, mod);
        if (!cur) {
                if (otherwise)
                        return d_strcpy(otherwise);

                return NULL;
        }

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if (cur_s && (cur_s->value))
                return d_strcpy(cur_s->value);
        else if (otherwise)
                return d_strcpy(otherwise);

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
        if (!cur)
                return otherwise;

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if (cur_s && (cur_s->value))
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
        if (!cur)
                return otherwise;

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if (cur_s && (cur_s->value))
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
bool get_bool_key(char *mod, char *key, bool otherwise)
{
        struct cfg *cur;
        struct setting *cur_s;
        
        cur = find_node(cfg_ls, &find_mod, mod);
        if (!cur)
                return otherwise;

        cur_s = find_node(cur->setting_ls, &find_key, key);
        if (cur_s && (cur_s->value)) {
                if (IS_TRUE(cur_s->value[0]))
                        return true;
                else if (IS_FALSE(cur_s->value[0]))
                        return false;
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
        const char *format[5];     /* sscanf formats */

        cfg_file = get_cfg_file();
        cfg_ls = init_list();      /* initialize our cfg list */

        mod[0] = '\0';
        key[0] = '\0';
        value[0] = '\0';

        format[0] = " [%63s[a-zA-Z0-9_-]]";                 /* [mod]         */
        format[1] = " %63s[a-zA-Z0-9_-] = \"%127s[^\"]\" "; /* key = "value" */
        format[2] = " %63s[a-zA-Z0-9_-] = '%127s[^\']' ";   /* key = 'value' */
        format[3] = " %63s[a-zA-Z0-9_-] = %127s[^;\n] ";    /* key = value   */
        format[4] = " %63s[a-zA-Z0-9_-] ";                  /* key           */

        while ((fgets(str, MAX_LINE_SIZE, cfg_file)) != NULL) {
                if (is_comment(str)) {
                        //printf("Skipping comment [%s]\n\n", chomp(str));
                        continue;
                }

                if (sscanf(str, format[0], mod) == 1) {
                        add_mod(mod);
                        continue;
                }

                if (parse(str, format[1], 2, key, value)) { }
                else if (parse(str, format[2], 2, key, value)) { }
                else if (parse(str, format[3], 2, key, value))
                        trim_t(value);
                else if (sscanf(str, format[4], key) == 1) {
                        strncpy(value, "True", sizeof(value) - 1);
                        value[sizeof(value) - 1] = '\0';
                }

                /* values of "" or '' are interpreted as False */
                if (!strcmp(value, "\"\"") || !strcmp(value, "''")) {
                        strncpy(value, "False", sizeof(value) - 1);
                        value[sizeof(value) - 1] = '\0';
                }

                if (mod[0] && key[0] && value[0]) {
                        add_key(mod, key, value);
                        char *char_key = get_char_key(mod, key, "ERROR");
                        printf("added-> mod [%s] key [%s] value [%s]\n",
                               mod, key, value);
                        printf("  char [%s] int [%d] double [%f] bool [%d]\n\n",
                               char_key,
                               get_int_key(mod, key, -1),
                               get_double_key(mod, key, -1),
                               get_bool_key(mod, key, -1));
                        free(char_key);
                }

                key[0] = '\0';
                value[0] = '\0';
        }

        fclose(cfg_file);
}

bool parse(const char *str, const char *format, int n, ...)
{
        va_list ap;
        va_list aq;
        char *p;
        int i;
        bool success;

        va_start(ap, n);
        va_copy(aq, ap);

        if (vsscanf(str, format, ap) != n) {
                for (i = 0; i < n; i++) {
                        p = va_arg(aq, char *);
                        *p = '\0';
                }

                success = false;
        } else {
                success = true;
        }

        va_end(ap);
        va_end(aq);

        return success;
}

/** 
 * @brief Load configuration. We check for the local configuration, usually
 *        ~/.donkyrc, then if not found, we try to open the system wide
 *        configuration. If none are found, we blow this joint.
 *
 * @return Pointer to an open .donkyrc file, if successful.
 */
FILE *get_cfg_file(void)
{
        char *cfg_file_path;
        FILE *cfg_file;

        asprintf(&cfg_file_path, "%s/%s", getenv("HOME"), DEFAULT_CONF);

        cfg_file = fopen(cfg_file_path, "r");
        free(cfg_file_path);
        if (!cfg_file) {
                printf("Warning: ~/%s file not found.\n", DEFAULT_CONF);

                asprintf(&cfg_file_path,
                         "%s/%s", 
                         SYSCONFDIR,
                         DEFAULT_CONF_GLOBAL);

                cfg_file = fopen(cfg_file_path, "r");
                free(cfg_file_path);
                if (!cfg_file) {
                        fprintf(stderr,
                                "Error: %s/%s file not found.\n",
                                SYSCONFDIR,
                                DEFAULT_CONF_GLOBAL);
                        exit(EXIT_FAILURE);
                }
        }

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
void clear_settings(struct setting *cur_s)
{
        free(cur_s->key);
        free(cur_s->value);
}

