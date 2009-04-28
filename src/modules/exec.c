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
#include <stdio.h>
#include <stdlib.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "exec";

/* Required function prototypes. */
void module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_exec(char *args);
int get_execbar(char *args);

/* These run on module startup */
void module_init(void)
{
        module_var_add(module_name, "exec", "get_exec", 10.0, VARIABLE_STR);
        module_var_add(module_name, "execbar", "get_execbar", 10.0, VARIABLE_BAR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_exec(char *args)
{
        FILE *execp;
        char *pipe;
        size_t len;

        execp = popen(args, "r");
        if (!execp)
                return "n/a";

        pipe = NULL;
        len = 0;
        getline(&pipe, &len, execp);
        pclose(execp);
        if (len && pipe)
                return m_freelater(chomp(pipe));

        return "n/a";
}

int get_execbar(char *args)
{
        FILE *execp;
        char *pipe;
        size_t len;
        int value;

        execp = popen(args, "r");
        if (!execp)
                return 0;

        pipe = NULL;
        len = 0;
        getline(&pipe, &len, execp);
        pclose(execp);
        if (len && pipe) {
                value = atoi(pipe);
                free(pipe);
                return value;
        }

        return 0;
}

