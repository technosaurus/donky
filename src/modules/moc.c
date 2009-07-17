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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "moc";

/* Globals */
static char *home;

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "moc", "get_moc", 10.0, VARIABLE_STR | ARGSTR);
        home = strdup(getenv("HOME"));
}

/* These run on module unload */
void module_destroy(void)
{
        free(home);
}

char *get_moc(char *args)
{
        FILE *mocp;
        char pipe[160];
        char *mocp_line;

        asprintf(&mocp_line, "HOME=\"%s\" mocp -Q \"%s\"", home, args);

        mocp = popen(mocp_line, "r");
        free(mocp_line);
        if (mocp == NULL)
                return "Could not query moc.";

        if (fgets(pipe, sizeof(pipe), mocp) == NULL) {
                pclose(mocp);
                return "No music playing.";
        }

        pclose(mocp);

        return m_strdup(chomp(pipe));
}

