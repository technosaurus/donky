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

#include "develop.h"
#include "../mem.h"

/* Module name */
char module_name[] = "exec";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_exec(char *args);

/* Globals */
char *ret_exec = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "exec", "get_exec", 10.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_exec(char *args)
{
        FILE *execp;
        
        char pipe[160];
        
        ret_exec = NULL;

        execp = popen(args, "r");
        if (execp == NULL)
                return m_strdup("n/a");

        if (fgets(pipe, 160, execp) == NULL) {
                pclose(execp);
                return m_strdup("n/a");
        }

        ret_exec = m_strdup(pipe);
        chomp(ret_exec);

        pclose(execp);

        return ret_exec;
}

