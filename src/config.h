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

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

struct cfg {
        char *mod;

        struct list *setting_ls;
};

/* function prototypes */
char *get_char_key(char *mod, char *key, char *otherwise);
int get_int_key(char *mod, char *key, int otherwise);
double get_double_key(char *mod, char *key, double otherwise);
bool get_bool_key(char *mod, char *key, bool otherwise);
void parse_cfg(void);
void clear_cfg(struct cfg *cur);

#endif /* CONFIG_H */

