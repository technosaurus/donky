/**
 * The CC0 1.0 Universal is applied to this work.
 *
 * To the extent possible under law, Matt Hayes and Jake LeMaster have
 * waived all copyright and related or neighboring rights to donky.
 * This work is published from the United States.
 *
 * Please see the copy of the CC0 included with this program for complete
 * information including limitations and disclaimers. If no such copy
 * exists, see <http://creativecommons.org/publicdomain/zero/1.0/legalcode>.
 */

#ifndef UTIL_H
#define UTIL_H

#include "../config.h"

#include <stddef.h>

/** 
 * @brief Debugging printf. This must be used with double parenthesis due to
 *        the limitations of ANSI C not supporting variadic macros.
 *              Example:
 *                      DEBUG(("blah blah blah, %s", str));
 */
#ifdef ENABLE_DEBUGGING
#define DEBUGF(args) printf args
#else
#define DEBUGF(args)
#endif /* ENABLE_DEBUGGING */

/**
 * @brief Free a pointer, then set it to NULL.
 */
#define freenull(ptr)           \
        do {                    \
                free(ptr);      \
                (ptr) = NULL;   \
        } while (0)

char *trim(char *str);
char *trim_l(char *str);
void trim_t(char *str);
int is_comment(char *str);
int is_all_spaces(char *str);
char *chop(char *str);
char *chomp(char *str);
char *substr(char *str, int offset, size_t len);
double get_time(void);
char *bytes_to_bigger(long double bytes);
int random_range(int min, int max);
unsigned int get_str_sum(const char *str);
void strfcpy(char *dst, const char *src, size_t siz);
void strfcat(char *dst, const char *src, size_t siz);
unsigned long int get_unix_time(void);

#endif /* UTIL_H */
