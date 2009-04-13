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

/* function prototypes */
char *get_char_key(char *mod, char *key);
int get_int_key(char *mod, char *key);
double get_double_key(char *mod, char *key);
int get_bool_key(char *mod, char *key);
void parse_cfg (void);
void clear_cfg(void);

/* holds [text] */
char *cfg_text;

#endif /* CONFIG_H */

