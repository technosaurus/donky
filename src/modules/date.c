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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "develop.h"

char module_name[] = "date_shet"; /* Up to 63 characters, any more and it will
                                     be truncated!  Doesn't matter though, just
                                     needs to be a some-what unique name. */

/* Required function prototypes. */
void module_init(void);
void module_destroy(void);

/* My function prototypes. */
char *get_date(char *);

/* Globals. */
char *ret_value = NULL;

/**
 * @brief This is run on module initialization.
 */
void module_init(void)
{
        module_var_add(module_name, "date", "get_date", VARIABLE_STR);
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
        if (ret_value)
                free(ret_value);
}

/**
 * @brief Get the current time in a custom format.
 *
 * @param args Argument string.
 *
 * @return Formatted time string
 */
char *get_date(char *args)
{
        if (ret_value)
                free(ret_value);
        
        char outstr[200];
        time_t t;
        struct tm *tmp;

        t = time(NULL);
        tmp = localtime(&t);

        if (tmp == NULL) {
                ret_value = strdup("n/a");
                return ret_value;
        }

        if (strftime(outstr, sizeof(outstr), args, tmp) == 0) {
                ret_value = strdup("n/a");
                return ret_value;
        }

        ret_value = strdup(outstr);
        return ret_value;
}