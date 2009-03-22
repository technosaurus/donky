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
#include <string.h>
#include <sys/time.h>

#include "util.h"

char *trim(char *);
char *trim_l(char *);
void trim_t(char *);
int is_comment(char *);
int is_all_spaces(char *);
char *chop(char *);
char *chomp(char *);
char *substr(char *, int, int);
double get_time(void);
void freeif(void *ptr);
char *d_strncpy(const char *str, int n);

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
 * @brief Free a pointer if it is not NULL.
 */
void freeif(void *ptr)
{
        if (ptr)
                free(ptr);
}

/** 
 * @brief A quicker alternative to strndup
 * 
 * @param str String to duplicate
 * @param n Number of chars to duplicate
 * 
 * @return Duplicated string
 */
char *d_strncpy(const char *str, int n)
{
        char *newstr = malloc((n + 1) * sizeof(char));
        strncpy(newstr, str, (n * sizeof(char)));
        newstr[n] = '\0';
        return newstr;
}

