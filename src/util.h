/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

/* these are for certain macros to look less ugly */
#define start do {
#define end   } while (0)

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
bool is_comment(char *);
bool is_all_spaces(char *);
char *chop(char *);
char *chomp(char *);
char *substr(char *, int, int);
double get_time(void);
char *d_strcpy(const char *str);
char *d_strncpy(const char *str, size_t n);
char *bytes_to_bigger(unsigned long bytes);
bool csscanf(const char *str, const char *format, int n, ...);
int random_range(int min, int max);
int create_tcp_listener(char *host, char *port);
ssize_t sendcrlf(int sock, const char *format, ...);

#endif /* UTIL_H */
