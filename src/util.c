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

#include <string.h>

#include "util.h"

char *trim(char *);
char *trim_l(char *);
void trim_t(char *);
int is_comment(char *);

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

        else
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
