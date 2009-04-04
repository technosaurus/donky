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
#include "../util.h"
#include "../mem.h"

char module_name[] = "date_shet"; /* Up to 63 characters, any more and it will
                                     be truncated!  Doesn't matter though, just
                                     needs to be a some-what unique name. */

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes. */
char *get_date(char *args);

/**
 * @brief This is run on module initialization.
 */
int module_init(void)
{
        module_var_add(module_name, "date", "get_date", 1.0, VARIABLE_STR);
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
        
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
        char ret_value[96];
        time_t t;
        struct tm *tmp;

        t = time(NULL);
        tmp = localtime(&t);

        if (tmp == NULL)
                return m_strdup("n/a");

        if (strftime(ret_value,
                     sizeof(ret_value) - sizeof(char),
                     args, tmp) == 0)
                return m_strdup("n/a");

        return m_strdup(ret_value);
}
