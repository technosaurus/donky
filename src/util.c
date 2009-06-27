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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "util.h"

static char *reverse_string(char *start, char *end);

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
int is_comment(char *str)
{
        str = trim_l(str);
        if (*str == ';')
                return 1;

        return 0;
}

/**
 * @brief Is this string all spaces?
 *
 * @param str String to check
 *
 * @return 1 for yes, 0 for no
 */
int is_all_spaces(char *str)
{
        unsigned int i;

        for (i = 0; i < strlen(str); i++)
                if (!isspace(str[i]))
                        return 0;

        return 1;
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
char *substr(char *str, int offset, size_t len)
{
        size_t siz = len + sizeof(char);
        char *dup = malloc(siz);
        str += offset;
        strfcpy(dup, str, siz);
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
 * @brief Convert raw bytes into formatted values.
 *
 * @param bytes Total bytes
 *
 * @return Formatted string
 */
#define KILO 1024.0
#define MEGA 1048576.0
#define GIGA 1073741824.0
char *bytes_to_bigger(unsigned long bytes)
{
        char str[16];

        if (bytes < KILO) {
                uint_to_str(str, bytes, sizeof(str));
                strfcat(str, "B", sizeof(str));
        } else if (bytes < MEGA) {
                float_to_str(str, bytes / KILO, 2, sizeof(str));
                strfcat(str, "KiB", sizeof(str));
        } else if (bytes < GIGA) {
                float_to_str(str, bytes / MEGA, 2, sizeof(str));
                strfcat(str, "MiB", sizeof(str));
        } else {
                float_to_str(str, bytes / GIGA, 2, sizeof(str));
                strfcat(str, "GiB", sizeof(str));
        }

        return dstrdup(str);
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
        srand((unsigned int) seconds);
        random_number = (rand() % (max - min + 1)) + min;

        return random_number;
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
static char *reverse_string(char *start, char *end)
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

/**
 * @brief Compare two strings case insensitively. Note: I return -1 if
 *        not equal rather than the greater than less than crap. 0 means
 *        equal.
 *        In other words, this does NOT conform to the de facto standard
 *        strcasecmp, so don't misuse it.
 *
 * @param s1 string 1
 * @param s2 string 2
 *
 * @return -1 for no match, 0 for match
 */
int dstrcasecmp(const char *s1, const char *s2)
{
        for (; (*s1 || *s2); s1++, s2++)
                if (tolower(*s1) != tolower(*s2))
                        return -1;
 
        return 0;
}

/**
 * @brief Copy src into dst, with a maximum of siz - 1 characters.  Guaranteed
 *        to be NUL terminated.  This has no truncation checking or any of that
 *        other pansy stuff.  Intended for hardcore c0d3rs only.  The 'f' stands
 *        for fast and furious.
 *
 * @param dst Destination string
 * @param src Source string
 * @param siz Size of destination string
 */
void strfcpy(char *dst, const char *src, size_t siz)
{
        size_t n = siz;

        /* Copy as many bytes as will fit */
        while (--n > 0)
                if ((*dst++ = *src++) == '\0')
                        return;

        /* Not enough room in dst, add NUL */
        if ((n == 0) && (siz != 0))
                *dst = '\0';
}

/**
 * @brief Concatenate src with dst, with a maximum of siz - 1 characters.
 *        Guaranteed to be NUL terminated.  This has no truncation checking or
 *        any of that other pansy stuff.  Intended for hardcore c0d3rs only.
 *        The 'f' stands for fast and furious.
 *
 * @param dst Destination string
 * @param src Source string
 * @param siz Size of destination string
 */
void strfcat(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        size_t n = siz;

        /* Find the end of dst and adjust bytes left but don't go past end */
        while ((n-- != 0) && (*d != '\0'))
                d++;

        n = siz - (d - dst);
        if (n == 0)
                return;

        while (*src != '\0') {
                if (n != 1) {
                        *d++ = *src;
                        n--;
                }
                src++;
        }

        *d = '\0';
}

/**
 * @brief A quick, portable strdup
 *
 * @param str String to duplicate
 *
 * @return Duplicated, malloc'd string
 */
char *dstrdup(const char *str)
{
        size_t siz = strlen(str) + sizeof(char);
        char *dup = malloc(siz);
        char *p;

        for (p = dup; siz != 0; siz--)
                *p++ = *str++;

        return dup;
}

/**
 * The following stra functions receive either a malloc'd or NULL ptr (dst)
 * which will be (re)allocated as necessary to hold the contents of src. These
 * functions never create something newly malloc'd unless you send them a NULL
 * dst ptr.
 *
 * If src is NULL when calling stra(n)cpy, dst will be free'd and set to NULL.
 * If src is NULL is calling stra(n)cat, dst is unaffected.
 *
 * The purposes of these functions are to eliminate the possibility of
 * truncation and buffer overflows and to remove the need for specifying
 * arbitrary buffer sizes - like when dealing with lame shit like path names.
 *
 * All functions return 0 on success, and -1 on a realloc() error.
 */

/** 
 * @brief Copy src to dst.
 * 
 * @return 0 on success, -1 on realloc() error.
 */
int stracpy(char **dst, const char *src)
{
        char *d;
        size_t srclen;

        if (src == NULL) {
                free(*dst);
                *dst = NULL;
                return 0;
        }

        srclen = strlen(src);
        if ((*dst == NULL) || (strlen(*dst) < srclen)) {
                *dst = realloc(*dst, srclen + sizeof(char));
                if (*dst == NULL)
                        return -1;
        }

        for (d = *dst; srclen != 0; srclen--)
                *d++ = *src++;
        *d = '\0';

        return 0;
}

/** 
 * @brief Concatenate src onto dst.
 * 
 * @return 0 on success, -1 on realloc() error.
 */
int stracat(char **dst, const char *src)
{
        char *d;
        size_t dstlen, srclen;
        size_t siz;

        if (src == NULL)
                return 0;
        else if (*dst == NULL)
                return stracpy(dst, src);

        dstlen = strlen(*dst);
        srclen = strlen(src);
        siz = dstlen + srclen + sizeof(char);
        if ((*dst = realloc(*dst, siz)) == NULL)
                return -1;

        d = *dst + dstlen;
        siz -= dstlen;
        while (--siz != 0)
                *d++ = *src++;

        *d = '\0';

        return 0;
}

/** 
 * @brief Copy n characters from src to dst.
 * 
 * @return 0 on success, -1 on realloc() error.
 */
int strancpy(char **dst, const char *src, size_t n)
{
        char *d;
        size_t srclen;
        size_t siz;

        if (src == NULL) {
                free(*dst);
                *dst = NULL;
                return 0;
        }

        srclen = strlen(src);
        if (n > srclen)
                siz = srclen + sizeof(char);
        else
                siz = n + sizeof(char);

        if ((*dst == NULL) || (strlen(*dst) < srclen))
                if ((*dst = realloc(*dst, siz)) == NULL)
                        return -1;

        d = *dst;
        while (--siz != 0)
                *d++ = *src++;

        *d = '\0';

        return 0;
}

/** 
 * @brief Concatenate n characters from src onto dst.
 * 
 * @return 0 on success, -1 on realloc() error.
 */
int strancat(char **dst, const char *src, size_t n)
{
        char *d;
        size_t dstlen, srclen;
        size_t siz;

        if (src == NULL)
                return 0;
        else if (*dst == NULL)
                return strancpy(dst, src, n);

        dstlen = strlen(*dst);
        srclen = strlen(src);
        if (n > srclen)
                siz = dstlen + srclen + sizeof(char);
        else
                siz = dstlen + n + sizeof(char);

        if ((*dst = realloc(*dst, siz)) == NULL)
                return -1;

        d = *dst + dstlen;
        siz -= dstlen;
        while (--siz != 0)
                *d++ = *src++;

        *d = '\0';

        return 0;
}

/**
 * @brief Get the current UNIX time.
 *
 * @return UNIX time
 */
unsigned long int get_unix_time(void)
{
        time_t t1;
        time_t t2;
        struct tm tms_utime;
        struct tm *tms_now;
        long int ret;

        tms_utime.tm_sec = 0;
        tms_utime.tm_min = 0;
        tms_utime.tm_hour = 0;
        tms_utime.tm_mday = 1;
        tms_utime.tm_mon = 0;
        tms_utime.tm_year = 70;
        tms_utime.tm_yday = 0;
        
        t2 = mktime(&tms_utime);

        t1 = time(NULL);
        tms_now = gmtime(&t1);
        t1 = mktime(tms_now);

        ret = (long int) difftime(t1, t2);

        return ret;
}

