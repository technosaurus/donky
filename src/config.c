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
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.a
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "util.h"

#define MAX_LINE_SIZE 1024

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
} *first_cfg = NULL, *last_cfg = NULL;

/* function prototypes */
void add_mod(char *mod);
struct cfg *find_mod(char *mod);
void add_key(char *mod, char *key, char *value);
char *get_char_key(char *mod, char *key);
int get_int_key(char *mod, char *key);
double get_double_key(char *mod, char *key);
int get_bool_key(char *mod, char *key);
void parse_cfg (void);
void config_clear(void);

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

        if (last_cfg) {
                last_cfg->next = new_mod;
                last_cfg = new_mod;
        }
        
        else {
                new_mod->first_setting = NULL;
                new_mod->last_setting = NULL;
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
                if (!strcmp(cur->mod, mod))
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
                return; // @fixme

        struct setting *new_set = malloc(sizeof(struct setting));
        
        new_set->key = key;
        new_set->value = value;

        new_set->next = NULL;

        if (cur->last_setting) {
                cur->last_setting->next = new_set;
                cur->last_setting = new_set;
        }

        else
                cur->first_setting = cur->last_setting = new_set;
}

/** 
 * @brief Get the char value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return A pointer to the char value of key
 */
char *get_char_key(char *mod, char *key)
{
        struct cfg *cur;
        
        cur = find_mod(mod);

        if (!cur)
                return NULL;

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcmp(cur_set->key, key))
                        return cur_set->value;

                cur_set = cur_set->next;
        }

        return NULL;
}

/** 
 * @brief Get the int value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return The int value of key
 */

int get_int_key(char *mod, char *key)
{
        struct cfg *cur = find_mod(mod);

        if (!cur)
                return -1;

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcmp(cur_set->key, key))
                        return strtol(cur_set->value, NULL, 0);

                cur_set = cur_set->next;
        }

        return -1;
}

/** 
 * @brief Get the double value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return The double value of key
 */

double get_double_key(char *mod, char *key)
{
        struct cfg *cur = find_mod(mod);

        if (!cur)
                return -1;

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcmp(cur_set->key, key))
                        return atof(cur_set->value);

                cur_set = cur_set->next;
        }

        return -1;
}

/** 
 * @brief Get the boolean value of a key of a mod
 * 
 * @param mod Name of mod
 * @param key Name of key
 * 
 * @return The boolean value of key
 */

int get_bool_key(char *mod, char *key)
{
        struct cfg *cur = find_mod(mod);

        int b;

        if (!cur)
                return -1;

        struct setting *cur_set = cur->first_setting;

        while (cur_set) {
                if(!strcmp(cur_set->key, key)) {
                        if (IS_TRUE(cur_set->value[0]))
                                b = 1;
                        else if (IS_FALSE(cur_set->value[0]))
                                b = 0;
                        else
                                b = -1;

                        return b;
                }

                cur_set = cur_set->next;
        }

        return -1;
}

/** 
 * @brief Parses the main donky configuration file
 */
void parse_cfg(void)
{
        char *cfg_file_path = NULL;
        cfg_file_path = getenv("HOME");
        cfg_file_path = strcat(cfg_file_path, "/.donkyrc");

        FILE *cfg_file = fopen(cfg_file_path, "r");

        if (cfg_file == NULL) {
                printf("Error: ~/.donkyrc file not found.\n");
                exit(EXIT_FAILURE);
        }

        /* these will hold/point to lines in .donkyrc */
        char str[MAX_LINE_SIZE];
        char *p;
        
        /* these will hold successfully parsed... stuff */
        char *mod = NULL;
        char *key = NULL;
        char *value = NULL;

        /* used by [text] */
        config_text = NULL;

        while (fgets(str, MAX_LINE_SIZE, cfg_file) != NULL) {

                if (is_comment(str)) /* let comment lines pass */
                        continue;

                /* scan for [mods] - don't add_mod if [text] */
                if (sscanf(str, " [%a[a-zA-Z0-9_-]]", &mod) == 1) {
                        if (strcmp(mod, "text") != 0)
                                add_mod(mod);
                        continue;
                }

                /* handle [text] - store all lines 'til next [mod] into config_text */
                if (!strcmp(mod, "text")) {
                        if (!config_text)
                                config_text = strndup(str, (strlen(str) * sizeof(char)));
                        else {
                                /* resize config_text so we can add more to it */
                                config_text = realloc(config_text, ((strlen(config_text) + strlen(str) + 2) * sizeof(char)));
                                strncat(config_text, str, (strlen(str) * sizeof(char)));
                        }
                        continue;
                }

                /* scan lines for keys and their values */
                else if (sscanf(str, " %a[a-zA-Z0-9_-] = \"%a[^\"]\" ", &key, &value) == 2) { }
                else if (sscanf(str, " %a[a-zA-Z0-9_-] = '%a[^\']' ", &key, &value) == 2) { }
                else if (sscanf(str, " %a[a-zA-Z0-9_-] = %a[^;\n] ", &key, &value) == 2)
                        trim_t(value);
                else if (sscanf(str, " %a[a-zA-Z0-9_-] ", &key) == 1)
                        value = strndup("True", (sizeof(char) * 4));
                
                /* if the value is "" or '', set it to False */
                if (value && (!strcmp(value, "\"\"") || !strcmp(value, "''")))
                        value = strndup("False", (sizeof(char) * 5));

                /* if we have all required ingredients, make an entry */
                if (mod && key && value) {
                        add_key(mod, key, value);
                        printf("added-> mod: %s || key: %s || value: %s ||\n", mod, key, value);
                        printf("char: %s || int: %d || double: %f || bool: %d ||\n\n",
                                get_char_key(mod, key), get_int_key(mod, key),
                                get_double_key(mod, key), get_bool_key(mod, key));
                }

                /* reset pointers */
                value = NULL;
                key = NULL;
        }

        fclose(cfg_file);
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

                        free(cur_setting->key);
                        free(cur_setting->value);
                        free(cur_setting);

                        cur_setting = next_setting;
                }

                free(cur_cfg->mod);
                free(cur_cfg);

                cur_cfg = next_cfg;
        }
}
