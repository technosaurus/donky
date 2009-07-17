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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

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
#define TERA 109951162776.0
char *bytes_to_bigger(long double bytes)
{
        char str[16];

        if (bytes < KILO)
                snprintf(str, sizeof(str), "%.2LfB", bytes);
        else if (bytes < MEGA)
                snprintf(str, sizeof(str), "%.2LfKiB", bytes / KILO);
        else if (bytes < GIGA)
                snprintf(str, sizeof(str), "%.2LfMiB", bytes / MEGA);
        else if (bytes < TERA)
                snprintf(str, sizeof(str), "%.2LfGiB", bytes / GIGA);
        else
                snprintf(str, sizeof(str), "%.2LfTiB", bytes / TERA);

        return strdup(str);
}

/**
 * @brief Return a random number between min and max.
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
 */
unsigned int get_str_sum(const char *str)
{
        unsigned int sum;

        for (sum = 0; *str != '\0'; str++)
                sum += *str;

        return sum;
}

/**
 * @brief Copy src into dst, with a maximum of siz - 1 characters.  Guaranteed
 *        to be NUL terminated.  This has no truncation checking or any of that
 *        other pansy stuff.  Intended for hardcore c0d3rs only.  The 'f' stands
 *        for fast and furious.
 */
void strfcpy(char *dst, const char *src, size_t siz)
{
        size_t n = siz;

        while (--n > 0)
                if ((*dst++ = *src++) == '\0')
                        return;

        if ((n == 0) && (siz != 0))
                *dst = '\0';
}

/**
 * @brief Concatenate src with dst, with a maximum of siz - 1 characters.
 *        Guaranteed to be NUL terminated.  This has no truncation checking or
 *        any of that other pansy stuff.  Intended for hardcore c0d3rs only.
 *        The 'f' stands for fast and furious.
 */
void strfcat(char *dst, const char *src, size_t siz)
{
        char *d = dst;
        size_t n = siz;

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
        unsigned long int ret;

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

        ret = (unsigned long int) difftime(t1, t2);

        return ret;
}

