/*
 * This file is part of donky.
 *
 * donky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * donky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        while(isspace(*str))
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
        int i;

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

        if (str[len] == '\n')
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
        str += offset;
        char *dup = malloc(length + 1);
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
        char str[32];
        char label[4];
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
        snprintf(str, sizeof(str),
                 "%.2f%s", recalc, label);

        return d_strcpy(str);
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
        va_list ap, aq;
        va_start(ap, n);
        va_copy(aq, ap);

        bool success = true;

        if (vsscanf(str, format, aq) != n) {
                int i;
                void **p;
                for (i = 0; i < n; i++) {
                        p = va_arg(ap, void **);
                        freenullif(*p);
                }

                success = false;
        }

        va_end(ap);
        va_end(aq);

        return success;
}

