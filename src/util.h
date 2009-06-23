/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef UTIL_H
#define UTIL_H

#include "../config.h"

#include <stddef.h>

#define DMAXPATHLEN 256

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
char *bytes_to_bigger(unsigned long bytes);
int random_range(int min, int max);
unsigned int get_str_sum(const char *str);
double pown(double x, double y);
char *uint_to_str(char *dst, unsigned long int src, size_t siz);
char *float_to_str(char *dst,
                   long double src,
                   unsigned int precision,
                   size_t siz);
int dstrcasecmp(const char *s1, const char *s2);
void strfcpy(char *dst, const char *src, size_t siz);
void strfcat(char *dst, const char *src, size_t siz);
char *dstrdup(const char *str);
int stracpy(char **dst, const char *src);
int stracat(char **dst, const char *src);
int strancpy(char **dst, const char *src, size_t n);
int strancat(char **dst, const char *src, size_t n);
unsigned long int get_unix_time(void);

#endif /* UTIL_H */
