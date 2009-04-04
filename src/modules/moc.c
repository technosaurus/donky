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
#include "../mem.h"

/* Module name */
char module_name[] = "moc";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_moc(char *args);

/* Globals */
char *ret_moc = NULL;
char home[MAXPATHLEN + 1];

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "moc", "get_moc", 10.0, VARIABLE_STR);
        snprintf(home, MAXPATHLEN, "%s", getenv("HOME"));
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_moc(char *args)
{
        FILE *mocp;
        
        char pipe[160];
        
        size_t size = strlen(args) + strlen(home) + (sizeof(char) * 19);
        char mocp_line[size];

        snprintf(mocp_line,
                 size,
                 "HOME=\"%s\" mocp -Q \"%s\"",
                 home, args);

        ret_moc = NULL;

        mocp = popen(mocp_line, "r");
        if (mocp == NULL)
                return m_strdup("Could not query moc.");

        if (fgets(pipe, 160, mocp) == NULL) {
                pclose(mocp);
                return m_strdup("No music playing.");
        }

        /* Copy one char less to remove the \n */
        ret_moc = m_strndup(pipe, (strlen(pipe) - sizeof(char)));

        pclose(mocp);

        return ret_moc;
}

