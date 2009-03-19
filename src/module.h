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
        VARIABLE_STR,
        VARIABLE_BAR,
        VARIABLE_CUSTOM
};

struct module {
        char name[64];
        void *handle;
        void *destroy;

        struct module *next;
};

struct module_var {
        char name[64];
        char method[64];
        enum variable_type type;
        void *sym;

        struct module *parent;
        struct module_var *next;
};

struct module_var *module_var_find(char *);
void module_load_all(void);
void clear_module(void);

#endif /* MODULE_H */
