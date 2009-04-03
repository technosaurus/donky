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

#ifndef MODULE_H
#define MODULE_H

enum variable_type {
        VARIABLE_STR,           /* Function should return char * */
        VARIABLE_BAR,           /* Function should return int between 0 and 100 */
        VARIABLE_GRAPH,         /* TBA */
        VARIABLE_CUSTOM,        /* TBA */
        VARIABLE_CRON           /* This is simply a method to be run on schedule. */
};

struct module {
        char name[64];          /* Unique identifier, value really doesn't matter. */
        void *handle;           /* Handle to the code in memory. */
        void *destroy;          /* Pointer to the module_destroy function. */

        struct module *next;    /* Next node in linked list. */
};

struct module_var {
        char name[64];                  /* Name of the variable. */
        char method[64];                /* Method name to call. */
        enum variable_type type;        /* Type of method.  See the enum above. */
        void *sym;                      /* Pointer to the function. */

        double timeout;                 /* Used for cron jobs */
        double last_update;             /* Ditto */

        struct module *parent;          /* Parent of this module. */
        struct module_var *next;        /* Next node in linked list. */
};

/* Function prototypes. */
int module_add(char *name, void *handle, void *destroy);
struct module *module_find(char *name);
int module_var_add(char *parent, char *name, char *method, double timeout, enum variable_type type);
struct module_var *module_var_find(char *name);
int module_load(char *path);
void module_unload(char *name);
void *module_get_sym(void *handle, char *name);
void module_load_all(void);
void clear_module(void);

#endif /* MODULE_H */

