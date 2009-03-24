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

#ifndef DEVELOP_H
#define DEVELOP_H

enum variable_type {
        VARIABLE_STR,           /* Function should return char * */
        VARIABLE_BAR,           /* Function should return int between 0 and 100 */
        VARIABLE_GRAPH,         /* TBA */
        VARIABLE_CUSTOM,        /* TBA */
        VARIABLE_CRON           /* This is simply a method to be run on schedule. */
};

extern int module_var_add(char *parent,
                          char *name,
                          char *method,
                          double timeout,
                          enum variable_type type);

#endif /* DEVELOP_H */
