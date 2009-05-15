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

#define _GNU_SOURCE
#include <ctype.h>
#include <math.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "util.h"

/**
 * @brief Trim leading and trailing whitespace from a string.
 *
 * @param str Character array pointer
 *
 * @return Haxified string
 */
char *trim(char *str)
{
        str = trim_l(str);
        trim_t(str);

        return str;
}

/**
 * @brief Trim leading whitespace only
 *
 * @param str Char pointer
 *
 * @return New offset location in string
 */
char *trim_l(char *str)
{
        while (isspace(*str))
                str++;

        return str;
}

/**
 * @brief Trim trailing whitespace only
 *
 * @param str Char pointer
 */
void trim_t(char *str)
{
        int i;

        for (i = strlen(str) - 1; i >= 0; i--) {
                if (!isspace(str[i])) {
                        str[i + 1] = '\0';
                        break;
                }
        }
}

/**
 * @brief Is the current string a comment?
 *
 * @param str String we're checking
 *
 * @return 1 if comment, 0 if not
 */
bool is_comment(char *str)
{
        str = trim_l(str);
        if (*str == ';')
                return true;

        return false;
}

/**
 * @brief Is this string all spaces?
 *
 * @param str String to check
 *
 * @return 1 for yes, 0 for no
 */
bool is_all_spaces(char *str)
{
        int i;

        for (i = 0; i < strlen(str); i++)
                if (!isspace(str[i]))
                        return false;

        return true;
}

/**
 * @brief Chop off the last character of a string.
 *
 * @param str Source string
 *
 * @return Pointer to string
 */
char *chop(char *str)
{
        str[strlen(str) - 1] = '\0';
        return str;
}

/**
 * @brief Chop off new line from string.
 *
 * @param str Source string
 *
 * @return Pointer to string
 */
char *chomp(char *str)
{
        int len = strlen(str) - 1;

        if (str[len] == '\n' || str[len] == '\r')
                str[len] = '\0';

        return str;
}

/**
 * @brief Return a substring of a string.
 *
 * @param str Source string
 * @param offset Offset to start at
 * @param length Length after offset to retrieve
 *
 * @return Substring of source string
 */
char *substr(char *str, int offset, int length)
{
        /* EFFICIENCY TESTED TO THE FUCKING MAXIMUM BY A MAN IN A LAB COAT */
        char *dup = malloc(length + 1);
        str += offset;
        strncpy(dup, str, length);
        dup[length] = '\0';
        return dup;
}

/**
 * @brief Get the current time in seconds, to a bunch of decimal places.
 *
 * @return Time in seconds
 */
double get_time(void)
{
        struct timeval timev;
        gettimeofday(&timev, NULL);
        return (double) timev.tv_sec + (((double) timev.tv_usec) / 1000000);
}

/**
 * @brief A quicker alternative to strdup
 *
 * @param str String to duplicate
 * @param n Number of chars to duplicate
 *
 * @return Duplicated string
 */
char *d_strcpy(const char *str)
{
        int n = strlen(str);
        char *newstr = malloc(n + sizeof(char));
        strncpy(newstr, str, n);
        newstr[n] = '\0';
        return newstr;
}

/**
 * @brief A quicker alternative to strndup
 *
 * @param str String to duplicate
 * @param n Number of chars to duplicate
 *
 * @return Duplicated string
 */
char *d_strncpy(const char *str, size_t n)
{
        char *newstr = malloc(n + sizeof(char));
        strncpy(newstr, str, n);
        newstr[n] = '\0';
        return newstr;
}

/**
 * @brief Convert raw bytes into formatted values.
 *
 * @param bytes Total bytes
 *
 * @return Formatted string
 */
char *bytes_to_bigger(unsigned long bytes)
{
        char *str;
        char label[4];
        int read;
        double tera = pow(1024, 4);
        double giga = pow(1024, 3);
        double mega = pow(1024, 2);
        double kilo = 1024;
        double recalc = 0.0;

        if (bytes >= tera) {
                recalc = bytes / tera;
                strncpy(label, "TiB", sizeof(label) - 1);
        } else if (bytes >= giga) {
                recalc = bytes / giga;
                strncpy(label, "GiB", sizeof(label) - 1);
        } else if (bytes >= mega) {
                recalc = bytes / mega;
                strncpy(label, "MiB", sizeof(label) - 1);
        } else if (bytes >= kilo) {
                recalc = bytes / kilo;
                strncpy(label, "KiB", sizeof(label) - 1);
        } else {
                recalc = bytes;
                strncpy(label, "B", sizeof(label) - 1);
        }

        label[3] = '\0';
        read = asprintf(&str, "%.2f%s", recalc, label);

        return (read != -1) ? str : NULL;
}

/**
 * @brief Conditional sscanf wrapper. If the amount of matches and assignments
 *        do not equal parameter int n, free and NULL anything that *was*
 *        matched and assigned. In other words, it's an all-or-nothing sscanf.
 *
 * @param str sscanf parameter
 * @param format sscanf parameter
 * @param n Number of arguments you're sending to be matched and assigned
 * @param ... sscanf parameter
 *
 * @return 1 if everything was matched and assigned successfully, else 0
 */
bool csscanf(const char *str, const char *format, int n, ...)
{
        va_list ap;
        va_list aq;
        void **p;
        bool success;
        int i;

        va_start(ap, n);
        va_copy(aq, ap);

        if (vsscanf(str, format, ap) != n) {
                for (i = 0; i < n; i++) {
                        p = va_arg(aq, void **);
                        freenullif(*p);
                }

                success = false;
        } else {
                success = true;
        }

        va_end(ap);
        va_end(aq);

        return success;
}

/**
 * @brief Return a random number between min and max.
 *
 * @param min
 * @param max
 *
 * @return
 */
int random_range(int min, int max)
{
        time_t seconds;
        long int random_number;

        if (max <= min)
                return min;

        time(&seconds);
        srandom((unsigned int)seconds);
        random_number = (random() % (max - min + 1)) + min;

        return random_number;
}

/**
 * @brief Create a TCP listening socket.
 *
 * @param host Hostname
 * @param port Port to listen on
 *
 * @return Socket!
 */
int create_tcp_listener(char *host, char *port)
{
        /* Code taken pretty much straight from the man page of getaddrinfo. */
        
        struct addrinfo hints;
        struct addrinfo *result, *rp;
        int sfd;
        int s;
        int opt = 1;

        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
        hints.ai_socktype = SOCK_STREAM; /* TCP socket */
        hints.ai_flags = AI_PASSIVE;     /* For wildcard IP address */
        hints.ai_protocol = 0;           /* Any protocol */
        hints.ai_canonname = NULL;
        hints.ai_addr = NULL;
        hints.ai_next = NULL;

        s = getaddrinfo(host, port, &hints, &result);
        if (s != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
                return -1;
        }

        /* getaddrinfo() returns a list of address structures.
         * Try each address until we successfully bind(2).
         * If socket(2) (or bind(2)) fails, we (close the socket
         * and) try the next address. */

        for (rp = result; rp != NULL; rp = rp->ai_next) {
                sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

                if (sfd == -1)
                        continue;

                if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
                        break;

                close(sfd);
        }

        if (rp == NULL) {
                fprintf(stderr, "Could not bind\n");
                return -1;
        }

        freeaddrinfo(result);

        /* Allow this to be reused (needed for reloads and such) */
        if ((setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                        &opt, sizeof(opt)) == -1)) {
                perror("setsockopt");
                close(sfd);
                return -1;
        }

        /* Start listening. */
        if ((listen(sfd, 10) == -1)) {
                perror("listen");
                close(sfd);
                return -1;
        }

        return sfd;
}

/**
 * @brief Send sprintf formatted string to socket, CR-LF appended.
 *
 * @param sock Socket
 * @param format Format string
 * @param ... Format arguments
 *
 * @return Bytes written to socket
 */
ssize_t sendcrlf(int sock, const char *format, ...)
{
        char buffer[1024];
	va_list ap;
        int n;
	
	va_start(ap, format);
	n = vsnprintf(buffer, sizeof(buffer) - 2, format, ap);
	va_end(ap);

	buffer[n] = '\r';
        buffer[n + 1] = '\n';

        return send(sock, buffer, n + 2, 0);
}

/**
 * @brief Return the sum of all characters in a string.
 *
 * @param str String to get sum of
 *
 * @return Sum
 */
unsigned int get_str_sum(char *str)
{
        unsigned int sum = 0;
        
        for (; *str; str++)
                sum += *str;

        return sum;
}
