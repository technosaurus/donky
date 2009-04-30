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

#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

/* these are for certain macros to look less ugly */
#define start do {
#define end        } while (0)

/** 
 * @brief Free a pointer if it exists.
 */
#define freeif(ptr)             \
start                           \
        if (ptr)                \
                free(ptr);      \
end

/** 
 * @brief Free and null a pointer.
 */
#define freenull(ptr)           \
start                           \
        free(ptr);              \
        (ptr) = NULL;           \
end

/** 
 * @brief Free and null a pointer if it exists.
 */
#define freenullif(ptr)         \
start                           \
        if (ptr)                \
                freenull(ptr);  \
end

char *trim(char *);
char *trim_l(char *);
void trim_t(char *);
int is_comment(char *);
int is_all_spaces(char *);
char *chop(char *);
char *chomp(char *);
char *substr(char *, int, int);
double get_time(void);
char *d_strcpy(const char *str);
char *d_strncpy(const char *str, size_t n);
char *bytes_to_bigger(unsigned long bytes);
bool csscanf(const char *str, const char *format, int n, ...);
int random_range(int min, int max);

#endif /* UTIL_H */
