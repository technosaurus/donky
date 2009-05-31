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

#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include "std/stdbool.h"
#include "std/string.h"

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
        unsigned int i;

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
        /* lol */
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
        char str[16];
        double tera = pown(1024, 4);
        double giga = pown(1024, 3);
        double mega = pown(1024, 2);
        double kilo = 1024;

        if (bytes >= tera) {
                float_to_str(str, bytes / tera, 2, sizeof(str));
                strlcat(str, "TiB", sizeof(str));
        } else if (bytes >= giga) {
                float_to_str(str, bytes / giga, 2, sizeof(str));
                strlcat(str, "GiB", sizeof(str));
        } else if (bytes >= mega) {
                float_to_str(str, bytes / mega, 2, sizeof(str));
                strlcat(str, "MiB", sizeof(str));
        } else if (bytes >= kilo) {
                float_to_str(str, bytes / kilo, 2, sizeof(str));
                strlcat(str, "KiB", sizeof(str));
        } else {
                uint_to_str(str, bytes, sizeof(str));
                strlcat(str, "B", sizeof(str));
        }

        return d_strcpy(str);
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
int create_tcp_listener(char *host, int port)
{
        int sfd;
        struct sockaddr_in server;
        struct hostent *hptr;
        int opt = 1;

        if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("socket");
                return -1;
        }

        if ((hptr = gethostbyname(host)) == NULL) {
                fprintf(stderr, "Could not gethostbyname(%s)\n", host);
                return -1;
        }
        memcpy(&server.sin_addr, hptr->h_addr_list[0], hptr->h_length);

        server.sin_family = AF_INET;
        server.sin_port = htons((short) port);
        server.sin_addr.s_addr = INADDR_ANY;

        if ((bind(sfd, (struct sockaddr *) &server, sizeof(server)) == -1)) {
                perror("bind");
                close(sfd);
                return -1;
        }

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
int sendcrlf(int sock, const char *format, ...)
{
        char buffer[2048];
	va_list ap;
	
	va_start(ap, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);

        return sendx(sock, "%s\r\n", buffer);
}

/**
 * @brief Send sprintf formatted string to socket
 *
 * @param sock Socket
 * @param format Format string
 * @param ... Format arguments
 *
 * @return Bytes written to socket
 */
int sendx(int sock, const char *format, ...)
{
        char buffer[2048];
	va_list ap;
        int n;
	
	va_start(ap, format);
	n = vsnprintf(buffer, sizeof(buffer) - 1, format, ap);
	va_end(ap);

        return send(sock, buffer, n, 0);
}

/**
 * @brief Return the sum of all characters in a string.
 *
 * @param str String to get sum of
 *
 * @return Sum
 */
unsigned int get_str_sum(const char *str)
{
        unsigned int sum = 0;
        
        for (; *str; str++)
                sum += *str;

        return sum;
}

/**
 * @brief This is just a rewrite of <math.h>'s pow function, which I'd like to
 *        stop using because of how I have to link in that stupid library.
 *
 * @param x Number
 * @param y Power
 *
 * @return Result
 */
double pown(double x, double y)
{
        double result;
        double i;

        result = x;

        for (i = 1.0; i < y; i += 1.0)
                result *= x;

        return result;
}

/** 
 * @brief Converts an unsigned int into a string. String is guaranteed
 *        to be null terminated.
 * 
 * @param dst Char pointer/array to fill
 * @param src The number to convert
 * @param siz Size of dst
 * 
 * @return Pointer to the new string
 */
char *uint_to_str(char *dst, unsigned long int src, size_t siz)
{
        char *str;
        unsigned long int tmp;
        size_t len;

        if ((dst == NULL) || (siz <= 1))
                return NULL;

        str = dst;
        len = 0;

        /* count how many digits src contains by dividing
         * it by increasing powers of 10 until it equals 0 */
        for (tmp = src; tmp != 0; len++)
                tmp /= 10;

        /* if dst is smaller than the number of digits in src, start dropping
         * digits off the end of src until it fits. afterward, len should equal
         * siz - 1 which will leave room for a '\0' at the end of dst */
        if (len >= siz)
                while (len >= siz) {
                        src /= 10;
                        len--;
                }
        else
                len++;

        /* convert the digits, one at a time */
        do {
                *str++ = 48 + (src % 10);
                src /= 10;
        } while (--len != 1); /* 1 byte left for terminating '\0' */

        /* null terminate */
        *str = '\0';

        /* reverse the string, because it's backward! shift str back
         * by 1 byte first so we don't move the terminating '\0' */
        return reverse_string(dst, --str);
}

/** 
 * @brief Converts a float into a string, deciminal places and all.
 *        String is guaranteed to be null terminated.
 * 
 * @param dst Char array/string to fill.
 * @param src Number to convert
 * @param precision Number of decimal places to write
 * @param siz Size of dst
 * 
 * @return Pointer to the new string
 */
char *float_to_str(char *dst,
                   long double src,
                   unsigned int precision,
                   size_t siz)
{
        char *str;
        long int num, tmp;
        unsigned int precision_check;
        size_t len;

        if ((dst == NULL) || (siz <= 3) || (precision == 0))
                return NULL;

        str = dst;
        num = src * pown(10, precision);
        precision_check = 0;
        len = 0;

        /* count how many digits src contains by dividing
         * it by increasing powers of 10 until it equals 0 */
        for (tmp = num; tmp != 0; len++)
                tmp /= 10;

        /* if dst is smaller than the number of digits in src, start dropping
         * digits off the end of src until it fits. afterward, len should equal
         * siz - 2 which leaves room for the decimal '.' and terminating '\0' */
        siz -= 2;
        if (len > siz)
                while (len > siz) {
                        num /= 10;
                        len--;
                }
        else
                len += 2;

        /* convert the digits, one at a time */
        do {
                /* have we added enough decimal places yet? */
                if (precision_check == precision) {
                        /* we don't add a decimal if it'd be the
                         * last char before the terminating '\0' */
                        if (len != 2) {
                                *str++ = '.';
                                precision_check++;
                        }
                        continue;
                } else if (precision_check < precision) {
                        precision_check++;
                }
                *str++ = 48 + (num % 10);
                num /= 10;
        } while (--len > 1); /* 1 byte left for terminating '\0' */

        /* null terminate */
        *str = '\0';

        /* reverse the string, because it's backward! shift str back
         * by 1 byte first so we don't move the terminating '\0' */
        return reverse_string(dst, --str);
}

/** 
 * @brief Reverses a string
 * 
 * @param start Pointer to the beginning of the string
 * @param end Pointer to the byte BEFORE the terminating '\0' of the string
 * 
 * @return Pointer to the beginning of the reversed string
 */
char *reverse_string(char *start, char *end)
{
        char rev;

        while (end > start) {
                rev = *end;
                *end-- = *start;
                *start++ = rev;
        }

        /* end is now 1 byte after the start of the string, so... */
        return (end - 1);
}

