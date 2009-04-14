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
#include "util.h"

#define IS_TRUE(c) ( \
        ((c) == 'y') || ((c) == 'Y') || \
        ((c) == 't') || ((c) == 'T') || \
        ((c) == '1') \
)

#define IS_FALSE(c) ( \
        ((c) == 'n') || ((c) == 'N') || \
        ((c) == 'f') || ((c) == 'F') || \
        ((c) == '0') \
)

struct setting {
        struct setting *next;

        char *key;
        char *value;
};

struct cfg {
        struct cfg *next;

        char *mod;

        struct setting *first_setting;
        struct setting *last_setting;
};

/* internal prototypes */
void add_mod(char *mod);
struct cfg *find_mod(char *mod);
void add_key(char *mod, char *key, char *value);

/* Globals */
struct cfg *first_cfg = NULL;
struct cfg *last_cfg = NULL;

/** 
 * @brief Add a mod to the configuration list
 * 
 * @param mod The name of the mod to add
 */
void add_mod(char *mod)
{
        struct cfg *new_mod = malloc(sizeof(struct cfg));

        new_mod->mod = mod;
        new_mod->next = NULL;
        new_mod->first_setting = NULL;
        new_mod->last_setting = NULL;

        if (last_cfg) {
                last_cfg->next = new_mod;
                last_cfg = new_mod;
        } else {
                first_cfg = last_cfg = new_mod;
        }
}

/** 
 * @brief Find a mod in the main configuration list
 * 
 * @param mod The name of the mod to find
 * 
 * @return Pointer to the found mod
 */
struct cfg *find_mod(char *mod)
{
        struct cfg *cur = first_cfg;

        while (cur) {
                if (!strcasecmp(cur->mod, mod))
                        return cur;

                cur = cur->next;
        }
        
        return NULL;
}

/** 
 * @brief Add a key (setting, option) and its value to its respective mod
 * 
 * @param mod Name of mod to add key to
 * @param key Name of key
 * @param value Value of the char key or NULL
 * @param int_value Value of the int key or NULL
 */
void add_key(char *mod, char *key, char *value)
{
        struct cfg *cur = find_mod(mod);

        if (!cur)
                return;

        struct setting *new_set = malloc(sizeof(struct setting));
        
        new_set->key = NULL;
        new_set->value = NULL;
        new_set->next = NULL;

        new_set->key = d_strcpy(key);
        new_set->value = d_strcpy(value);

        if (cur->last_setting) {
                cur->last_setting->next = new_set;
                cur->last_setting = new_set;
        } else {
                cur->first_setting = cur->last_setting = new_set;
        }
}

/** 
 * @brief Get the char value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return A pointer to the char value of key
 */
char *get_char_key(char *mod, char *key, char *otherwise)
{
        struct cfg *cur;
        
        cur = find_mod(mod);

        if (!cur) {
                if (!otherwise)
                        return NULL;
                else
                        return d_strcpy(otherwise);
        }

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcasecmp(cur_set->key, key))
                        return d_strcpy(cur_set->value);

                cur_set = cur_set->next;
        }

        if (!otherwise)
                return NULL;
        else
                return d_strcpy(otherwise);
}

/** 
 * @brief Get the int value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return The int value of key
 */
int get_int_key(char *mod, char *key, int otherwise)
{
        struct cfg *cur = find_mod(mod);

        if (!cur)
                return otherwise;

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcasecmp(cur_set->key, key))
                        return atoi(cur_set->value);

                cur_set = cur_set->next;
        }

        return otherwise;
}

/** 
 * @brief Get the double value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return The double value of key
 */
double get_double_key(char *mod, char *key, double otherwise)
{
        struct cfg *cur = find_mod(mod);

        if (!cur)
                return otherwise;

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcasecmp(cur_set->key, key))
                        return atof(cur_set->value);

                cur_set = cur_set->next;
        }

        return otherwise;
}

/** 
 * @brief Get the boolean value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return The boolean value of key
 */
int get_bool_key(char *mod, char *key, int otherwise)
{
        struct cfg *cur = find_mod(mod);

        if (!cur)
                return otherwise;

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcasecmp(cur_set->key, key)) {
                        if (IS_TRUE(cur_set->value[0]))
                                return 1;
                        else if (IS_FALSE(cur_set->value[0]))
                                return 0;
                        else
                                break;
                }

                cur_set = cur_set->next;
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

        if (cfg_file == NULL) {
                printf("Error: ~/.donkyrc file not found.\n");
                exit(EXIT_FAILURE);
        }

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
                if (sscanf(str, " [%a[a-zA-Z0-9_-]]", &mod) == 1) {
                        if (strcasecmp(mod, "text") != 0)
                                add_mod(mod);

                        continue;
                }

                /* handle [text] - store all lines 'til next [mod] into cfg_text */
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
                else if (sscanf(str, " %a[a-zA-Z0-9_-] = \"%a[^\"]\" ", &key, &value) == 2) { }
                else if (sscanf(str, " %a[a-zA-Z0-9_-] = '%a[^\']' ", &key, &value) == 2) { }
                else if (sscanf(str, " %a[a-zA-Z0-9_-] = %a[^;\n] ", &key, &value) == 2)
                        trim_t(value);
                else if (sscanf(str, " %a[a-zA-Z0-9_-] ", &key) == 1)
                        value = d_strncpy("True", (sizeof(char) * 4));
 
                /* if the value is "" or '', set it to False */
                if (value && (!strcmp(value, "\"\"") || !strcmp(value, "''"))) {
                        free(value);
                        value = d_strncpy("False", (sizeof(char) * 5));
                }

                /* if we have all required ingredients, make an entry */
                if (mod && key && value) {
                        add_key(mod, key, value);
                        char *chrkey = get_char_key(mod, key, "ERROR");
                        printf("added-> mod: %s || key: %s || value: %s ||\n", mod, key, value);
                        printf("char: %s || int: %d || double: %f || bool: %d ||\n\n",
                                chrkey, get_int_key(mod, key, -1),
                                get_double_key(mod, key, -1), get_bool_key(mod, key, -1));
                        free(chrkey);
                }

                /* free str & reset pointers */
                free(str);
                str = NULL;
                value = NULL;
                key = NULL;
        }

        /* Close file descriptor. */
        fclose(cfg_file);

        if (cfg_text == NULL) {
                printf("Config error: missing [text] section.");
                clear_cfg();
                exit(EXIT_FAILURE);
        }
}

/** 
 * @brief Free all cfg nodes from memory
 */
void clear_cfg(void)
{
        struct cfg *cur_cfg = first_cfg;
        struct cfg *next_cfg;
        
        struct setting *cur_setting;
        struct setting *next_setting;

        while (cur_cfg) {
                next_cfg = cur_cfg->next;
                
                cur_setting = cur_cfg->first_setting;

                while (cur_setting) {
                        next_setting = cur_setting->next;

                        freeif(cur_setting->key);
                        freeif(cur_setting->value);
                        freeif(cur_setting);

                        cur_setting = next_setting;
                }

                freeif(cur_cfg->mod);
                freeif(cur_cfg);

                cur_cfg = next_cfg;
        }

        first_cfg = NULL;
        last_cfg = NULL;
}
