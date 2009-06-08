/*
 * Functions dstrlcpy and dstrlcat are adapted from OpenBSD's
 * libc functions strlcpy and strlcat:
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *      $OpenBSD: strlcat.c,v 1.13 2005/08/08 08:05:37 espie Exp $
 *      $OpenBSD: strlcpy.c,v 1.11 2006/05/05 15:27:38 millert Exp $
 *
 * Everything else:
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

#include <stddef.h>

#define DMAXPATHLEN 256

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
char *dstrdup(const char *str);
size_t dstrlcpy(char *dst, const char *src, size_t siz);
size_t dstrlcat(char *dst, const char *src, size_t siz);
int dstrcasecmp(const char *s1, const char *s2);
#endif /* UTIL_H */

