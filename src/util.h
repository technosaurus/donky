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

/**
 * @brief Free a pointer if it isn't NULL.
 */
#define freeif(ptr) \
        do { \
                if (ptr) \
                        free(ptr); \
        } while (0)

/** 
 * @brief Free and NULL a pointer.
 */
#define freenull(ptr) \
        do { \
                free(ptr); \
                (ptr) = NULL; \
        } while (0)

/** 
 * @brief Free and NULL a pointer if it isn't NULL.
 */
#define freenullif(ptr) \
        do { \
                if (ptr) \
                        freenull(ptr); \
        } while (0)

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
char *d_strncpy(const char *str, int n);
char *bytes_to_bigger(unsigned long bytes);
bool csscanf(const char *str, const char *format, int n, ...);

#endif /* UTIL_H */
