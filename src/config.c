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
#include "lists.h"
#include "util.h"

#define IS_TRUE(c) ( ((c) == 'y') || ((c) == 'Y') || \
                     ((c) == 't') || ((c) == 'T') || \
                     ((c) == '1') )

#define IS_FALSE(c) ( ((c) == 'n') || ((c) == 'N') || \
                      ((c) == 'f') || ((c) == 'F') || \
                      ((c) == '0') )

/* internal structs/prototypes */
struct setting {
        char *key;
        char *value;
};

void add_mod(char *mod);
int find_mod(struct cfg *cur, char *mod);
void add_key(char *mod, char *key, char *value);
int find_key(struct setting *cur, char *key);

void clear_settings(struct setting *cur_s);

/* Globals */
struct list *cfg_fl = NULL;

/** 
 * @brief Add a mod to the configuration list
 * 
 * @param mod The name of the mod to add
 */
void add_mod(char *mod)
{
        struct cfg *new_mod = malloc(sizeof(struct cfg));
        new_mod->mod = mod;
        new_mod->setting_fl = NULL;
        new_mod->setting_fl = init_list();

        add_node(cfg_fl, new_mod);
}

/** 
 * @brief Callback used in finding a mod in the coming functions.
 * 
 * @param cur cfg node whose mod we check against
 * @param mod Mod name we're looking for
 * 
 * @return 1 if this is the node we want, 0 if not
 */
int find_mod(struct cfg *cur, char *mod)
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
        struct cfg *cur = find_node(cfg_fl, &find_mod, mod);
        if (!cur)
                return;

        struct setting *new_set = malloc(sizeof(struct setting));
        new_set->key = NULL;
        new_set->key = d_strcpy(key);
        new_set->value = NULL;
        new_set->value = d_strcpy(value);

        add_node(cur->setting_fl, new_set);
}

/** 
 * @brief Callback used in finding a key in the coming functions.
 * 
 * @param cur setting node whose key we check against
 * @param mod Key name we're looking for
 * 
 * @return 1 if this is the node we want, 0 if not
 */
int find_key(struct setting *cur, char *key)
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
        struct cfg *cur = find_node(cfg_fl, &find_mod, mod);
        if (!cur) {
                if (otherwise)
                        return d_strcpy(otherwise);

                return NULL;
        }

        struct setting *cur_s = find_node(cur->setting_fl, &find_key, key);
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
        struct cfg *cur = find_node(cfg_fl, &find_mod, mod);
        if (!cur)
                return otherwise;

        struct setting *cur_s = find_node(cur->setting_fl, &find_key, key);
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
        struct cfg *cur = find_node(cfg_fl, &find_mod, mod);
        if (!cur)
                return otherwise;

        struct setting *cur_s = find_node(cur->setting_fl, &find_key, key);
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
int get_bool_key(char *mod, char *key, int otherwise)
{
        struct cfg *cur = find_node(cfg_fl, &find_mod, mod);
        if (!cur)
                return otherwise;

        struct setting *cur_s = find_node(cur->setting_fl, &find_key, key);
        if (cur_s && (cur_s->value)) {
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
        char *cfg_file_path = NULL;
        asprintf(&cfg_file_path, "%s/%s", getenv("HOME"), ".donkyrc");

        FILE *cfg_file = fopen(cfg_file_path, "r");
        free(cfg_file_path);
        if (!cfg_file) {
                printf("Error: ~/.donkyrc file not found.\n");
                exit(EXIT_FAILURE);
        }

        /* initialize our cfg list */
        cfg_fl = init_list();

        /* these will hold/point to lines in .donkyrc */
        char *str = NULL;
        size_t len = 0;

        /* these will hold successfully parsed... stuff */
        char *mod = NULL;
        char *key = NULL;
        char *value = NULL;

        /* used by [text] */
        cfg_text = NULL;

        while ((getline(&str, &len, cfg_file)) != -1) {
                /* Skip comments. */
                if (is_comment(str)) {
                        free(str);
                        str = NULL;
                        continue;
                }

                /* scan for [mods] - don't add_mod if [text] */
                if (sscanf(str, " [%a[a-zA-Z0-9_-]]", &mod)) {
                        if (strcasecmp(mod, "text") != 0)
                                add_mod(mod);

                        continue;
                }

                /* handle [text]: store all lines until the
                 * next [mod] into cfg_text */
                if (mod && !strcasecmp(mod, "text")) {
                        if (cfg_text == NULL) {
                                cfg_text = d_strcpy(str);
                        } else {
                                /* resize cfg_text so we can add more to it */
                                cfg_text = realloc(cfg_text,
                                                   strlen(cfg_text) +
                                                        strlen(str) +
                                                        sizeof(char));
                                strcat(cfg_text, str);
                        }

                        continue;
                }

                /* scan lines for keys and their values */
                else if (csscanf(str, " %a[a-zA-Z0-9_-] = \"%a[^\"]\" ", 2, &key, &value)) { }
                else if (csscanf(str, " %a[a-zA-Z0-9_-] = '%a[^\']' ", 2, &key, &value)) { }
                else if (csscanf(str, " %a[a-zA-Z0-9_-] = %a[^;\n] ", 2, &key, &value))
                        trim_t(value);
                else if (sscanf(str, " %a[a-zA-Z0-9_-] ", &key))
                        value = d_strcpy("True");
 
                /* if the value is "" or '', set it to False */
                if (value && (!strcmp(value, "\"\"") || !strcmp(value, "''"))) {
                        free(value);
                        value = d_strcpy("False");
                }

                /* if we have all required ingredients, make an entry */
                if (mod && key && value) {
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

                /* free str & reset pointers */
                free(str);
                str = NULL;
                value = NULL;
                key = NULL;
        }

        fclose(cfg_file);

        if (cfg_text == NULL) {
                printf("Config error: missing [text] section.");
                del_list(cfg_fl, &clear_cfg);
                exit(EXIT_FAILURE);
        }
}

/** 
 * @brief Frees the contents of the cfg struct passed to it.
 * 
 * @param cur cfg struct to clean
 */
void clear_cfg(struct cfg *cur)
{
        freeif(cur->mod);
        del_list(cur->setting_fl, &clear_settings);
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

