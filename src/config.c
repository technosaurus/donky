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
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX_LINE_SIZE 1024

union key_value {
        int int_value;
        char char_value[256];
};

struct setting {
        struct setting *next;
        
        char *key;
        union key_value value;
};

struct cfg {
        struct cfg *next;

        char *mod;

        struct setting *first_setting;
        struct setting *last_setting;
} *first_cfg = NULL, *last_cfg = NULL;

void add_mod(char *mod);
struct cfg *find_mod(char *mod);
void add_key(char *mod, char *key, char *char_value, int *int_value);
char *get_char_key(char *mod, char *key);
int get_int_key(char *mod, char *key);
void parse_cfg (void);

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
 * @param char_value Value of the char key or NULL
 * @param int_value Value of the int key or NULL
 */
void add_key(char *mod, char *key, char *char_value, int *int_value)
{
        struct cfg *cur = find_mod(mod);

        if (!cur)
                return; // @fixme

        struct setting *new_set = malloc(sizeof(struct setting));
        new_set->key = key;
        
        if (!char_value)
                new_set->value.int_value = *int_value;
        else
                strncpy(new_set->value.char_value, char_value, sizeof(new_set->value.char_value));
        
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
                        return cur_set->value.char_value;

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
                        return cur_set->value.int_value;

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
                exit(1);
        }

        char str[MAX_LINE_SIZE];

        char *mod = NULL;
        char *key;
        char *char_value;
        int *int_value = malloc(sizeof(int));

        config_text = NULL;

        while (fgets(str, MAX_LINE_SIZE, cfg_file) != NULL) {

                char_value = NULL; key = NULL; *int_value = 0;

                /*     [mod]                  */ /* don't add_mod if [text] */
                if (sscanf(str, " [%a[^]]] ", &mod) == 1) {
                        if (strcmp(mod, "text") != 0)
                                add_mod(mod);
                }

                /* handle [text] - store all lines 'til next [mod] into config_text */
                else if (!strcmp(mod, "text")) {
                        if (!config_text)
                                config_text = strndup(str, strlen(str));
                        else {
                                /* resize config_text so we can add more to it */
                                config_text = realloc(config_text, (strlen(config_text) + strlen(str)) + (2 * sizeof(char)));
                                strncat(config_text, str, strlen(str));
                        }
                }

                /*     key = "int_value"     */
                else if (sscanf(str, "%a[0-9a-zA-Z_] = \"%d\" \n", &key, int_value) == 2)
                        add_key(mod, key, char_value, int_value);

                /*     key = int_value       */
                else if (sscanf(str, "%a[0-9a-zA-Z_] = %d \n", &key, int_value) == 2 )
                        add_key(mod, key, char_value, int_value);

                /*     key = "char_value"    */
                else if (sscanf(str, "%a[0-9a-zA-Z_] = \"%a[^\"]\" \n", &key, &char_value) == 2)
                        add_key(mod, key, char_value, int_value);

                /*     key = char_value      */
                else if (sscanf(str, "%a[0-9a-zA-Z_] = %a[^\n]", &key, &char_value) == 2)
                        add_key(mod, key, char_value, int_value);

                /*     key                   */ /* assumes an int_value of 1 */
                else if (sscanf(str, " %a[0-9a-zA-Z_] \n", &key) == 1) {
                        *int_value = 1;
                        add_key(mod, key, char_value, int_value);
                }

                if (char_value)
                        free(char_value);

        }

        free(int_value);
        fclose(cfg_file);
}

