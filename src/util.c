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
        dstrlcpy(dup, str, siz);
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
char *bytes_to_bigger(unsigned long bytes)
{
        char str[16];
        double kilo = 1024;
        double mega = kilo * 1024;
        double giga = mega * 1024;
        double tera = giga * 1024;

        if (bytes >= tera) {
                float_to_str(str, bytes / tera, 2, sizeof(str));
                dstrlcat(str, "TiB", sizeof(str));
        } else if (bytes >= giga) {
                float_to_str(str, bytes / giga, 2, sizeof(str));
                dstrlcat(str, "GiB", sizeof(str));
        } else if (bytes >= mega) {
                float_to_str(str, bytes / mega, 2, sizeof(str));
                dstrlcat(str, "MiB", sizeof(str));
        } else if (bytes >= kilo) {
                float_to_str(str, bytes / kilo, 2, sizeof(str));
                dstrlcat(str, "KiB", sizeof(str));
        } else {
                uint_to_str(str, bytes, sizeof(str));
                dstrlcat(str, "B", sizeof(str));
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
        srandom((unsigned int)seconds);
        random_number = (random() % (max - min + 1)) + min;

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
 * @brief A quicker alternative to strdup
 *
 * @param str String to duplicate
 * @param n Number of chars to duplicate
 *
 * @return Duplicated string
 */
char *dstrdup(const char *str)
{
        size_t siz = strlen(str) + sizeof(char);
        char *newstr = malloc(siz);
        dstrlcpy(newstr, str, siz);
        return newstr;
}

/*
 * Copy src to string dst of size siz.  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz == 0).
 * Returns strlen(src); if retval >= siz, truncation occurred.
 */
size_t dstrlcpy(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        const char *s = src;
        size_t n = siz;
        size_t retval;

        /* Copy as many bytes as will fit */
        if (n != 0) {
                while (--n != 0) {
                        if ((*d++ = *s++) == '\0')
                                break;
                }
        }

        /* Not enough room in dst, add NUL and traverse rest of src */
        if (n == 0) {
                if (siz != 0)
                        *d = '\0';      /* NUL-terminate dst */
                while (*s++)
                        ;
        }

        /* count does not include NUL */
        retval = s - src - 1;

#ifdef ENABLE_DEBUG
        if (retval >= siz)
                fprintf(stderr, "dstrlcpy truncation detected. "
                                "Attempted to copy %d chars into "
                                "an array of size %d. [%s]\n",
                                retval, siz, dst);
#endif

        return retval;
}

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */
size_t dstrlcat(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        const char *s = src;
        size_t n = siz;
        size_t dlen;
        size_t retval;

        /* Find the end of dst and adjust bytes left but don't go past end */
        while (n-- != 0 && *d != '\0')
                d++;
        dlen = d - dst;
        n = siz - dlen;

        if (n == 0) {
                retval = dlen + strlen(s);
                goto out;
        }

        while (*s != '\0') {
                if (n != 1) {
                        *d++ = *s;
                        n--;
                }
                s++;
        }
        *d = '\0';

        /* count does not include NUL */
        retval = dlen + (s - src);

out:
#ifdef ENABLE_DEBUG
        if (retval >= siz)
                fprintf(stderr, "dstrlcat truncation detected. "
                                "Attempted to copy %d chars into "
                                "an array of size %d. [%s]\n",
                                retval, siz, dst);
#endif

        return retval;
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
void dstrfcpy(char *dst, const char *src, size_t siz)
{
        size_t n;
        n = siz;

        /* Copy as many bytes as will fit */
        while (--n > 0) {
                if ((*dst++ = *src++) == '\0')
                        break;
        }

        /* Not enough room in dst, add NUL */
        if (n <= 0) {
                if (siz != 0)
                        *dst = '\0';      /* NUL-terminate dst */
        }
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
void dstrfcat(char *dst, const char *src, size_t siz)
{
        char *d;
        size_t n;

        d = dst;
        n = siz;

        /* Find the end of dst and adjust bytes left but don't go past end */
        while (n-- != 0 && *d != '\0')
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
