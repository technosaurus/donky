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
#include <string.h>
#include <sys/param.h>

#include "develop.h"
#include "../util.h"

/* Module name */
char module_name[] = "moc";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_moc(char *args);

/* Globals */
char *ret_moc = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "moc", "get_moc", 10.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{
        freeif(ret_moc);
}

char *get_moc(char *args)
{
        FILE *mocp;
        char pipe[160];

        freeif(ret_moc);
        ret_moc = NULL;

        char home[MAXPATHLEN + 1];
        snprintf(home, MAXPATHLEN, "%s", getenv("HOME"));

        size_t size = strlen(home) + strlen(args) + (sizeof(char) * 19);
        char *mocp_line = malloc(size);
        snprintf(mocp_line, size, "HOME=\"%s\" mocp -Q \"%s\"", home, args);

        mocp = popen(mocp_line, "r");
        if (mocp == NULL) {
                free(mocp_line);
                ret_moc = d_strcpy("Could not query moc.");
                return ret_moc;
        }

        if (fgets(pipe, 160, mocp) == NULL) {
                free(mocp_line);
                pclose(mocp);
                ret_moc = d_strcpy("No music playing.");
                return ret_moc;
        }

        /* Copy one char less to remove the \n */
        ret_moc = d_strncpy(pipe, (strlen(pipe) - sizeof(char)));

        free(mocp_line);
        pclose(mocp);

        return ret_moc;
}

