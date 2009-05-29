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

#include "std/stdbool.h"

#define DMAXPATHLEN 256

#define freenull(ptr)           \
        do {                    \
                free(ptr);      \
                (ptr) = NULL;   \
        } while (0)

char *trim(char *);
char *trim_l(char *);
void trim_t(char *);
bool is_comment(char *);
bool is_all_spaces(char *);
char *chop(char *);
char *chomp(char *);
char *substr(char *, int, int);
double get_time(void);
char *d_strcpy(const char *str);
char *d_strncpy(const char *str, size_t n);
char *bytes_to_bigger(unsigned long bytes);
int random_range(int min, int max);
int create_tcp_listener(char *host, int port);
int sendcrlf(int sock, const char *format, ...);
unsigned int get_str_sum(const char *str);
double pown(double x, double y);
char *uint_to_str(char *dst, unsigned long int src, size_t siz);
char *float_to_str(char *dst,
                   long double src,
                   unsigned int precision,
                   size_t siz);
char *reverse_string(char *start, char *end);
#endif /* UTIL_H */

