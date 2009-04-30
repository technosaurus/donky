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

#define _GNU_SOURCE
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
static FILE *get_cfg_file(void);
static void clear_settings(struct setting *cur_s);

char *cfg_text;
struct list *cfg_ls;

#define IS_TRUE(c) ( ((c) == 'y') || ((c) == 'Y') || \
                     ((c) == 't') || ((c) == 'T') || \
                     ((c) == '1') )

#define IS_FALSE(c) ( ((c) == 'n') || ((c) == 'N') || \
                      ((c) == 'f') || ((c) == 'F') || \
                      ((c) == '0') )

/** 
 * @brief Add a mod to the configuration list
 * 
 * @param mod The name of the mod to add
 */
void add_mod(char *mod)
{
        struct cfg *new_mod;

        new_mod = malloc(sizeof(struct cfg));
        new_mod->mod = mod;
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
        char *str;
        size_t len;
        char *mod;
        char *key;
        char *value;
        const char *format[5];     /* sscanf formats */

        cfg_file = get_cfg_file();
        cfg_ls = init_list();      /* initialize our cfg list */
        cfg_text = NULL;           /* used in handling [text] */

        str = NULL;
        len = 0;

        mod = NULL;
        key = NULL;
        value = NULL;

        format[0] = " [%m[a-zA-Z0-9_-]]";              /* [mod]            */
        format[1] = " %m[a-zA-Z0-9_-] = \"%m[^\"]\" "; /* key = "value"    */
        format[2] = " %m[a-zA-Z0-9_-] = '%m[^\']' ";   /* key = 'value'    */
        format[3] = " %m[a-zA-Z0-9_-] = %m[^;\n] ";    /* key = value      */
        format[4] = " %m[a-zA-Z0-9_-] ";               /* key              */

        while ((getline(&str, &len, cfg_file)) != -1) {
                if (is_comment(str)) {
                        //printf("Skipping comment [%s]\n\n", chomp(str));
                        freenull(str);
                        continue;
                }

                if (sscanf(str, format[0], &mod) == 1) {
                        /* we don't add [text] to the cfg list */
                        if (strcasecmp(mod, "text") != 0)
                                add_mod(mod);
 
                        continue;
                }

                /* handle [text] here instead */
                if (mod && !strcasecmp(mod, "text")) {
                        if (!cfg_text) {
                                cfg_text = d_strcpy(str);
                        } else {
                                cfg_text = realloc(cfg_text,
                                                   strlen(cfg_text) +
                                                        strlen(str) +
                                                        sizeof(char));
                                strcat(cfg_text, str);
                        }

                        continue;
                }

                else if (csscanf(str, format[1], 2, &key, &value)) { }
                else if (csscanf(str, format[2], 2, &key, &value)) { }
                else if (csscanf(str, format[3], 2, &key, &value))
                        trim_t(value);
                else if (sscanf(str, format[4], &key) == 1)
                        value = d_strcpy("True");

                /* values of "" or '' are interpreted as False */
                if (value && (!strcmp(value, "\"\"") || !strcmp(value, "''"))) {
                        free(value);
                        value = d_strcpy("False");
                }

                if (mod && key && value) {
                        add_key(mod, key, value);
                        /*char *char_key = get_char_key(mod, key, "ERROR");
                        printf("added-> mod [%s] key [%s] value [%s]\n",
                               mod, key, value);
                        printf("  char [%s] int [%d] double [%f] bool [%d]\n\n",
                               char_key,
                               get_int_key(mod, key, -1),
                               get_double_key(mod, key, -1),
                               get_bool_key(mod, key, -1));
                        free(char_key);*/
                }

                freenull(str);
                freenullif(key);
                freenullif(value);
        }

        fclose(cfg_file);

        /* Set the default text section if none existed. */
        if (cfg_text == NULL)
                cfg_text = d_strcpy("${color red}ATTENTION TURD:$color Please" \
                                    " edit your donky configuration file and" \
                                    " add a ${color green}[text]$color" \
                                    " section. Do not disappoint me again.");
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
        freeif(cur->mod);
        del_list(cur->setting_ls, &clear_settings);
}

/** 
 * @brief Frees the contents of the setting struct passed to it.
 * 
 * @param cur_s setting struct to clean
 */
void clear_settings(struct setting *cur_s)
{
        freeif(cur_s->key);
        freeif(cur_s->value);
}

