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

#include "../mem.h"
#include "../module.h"
#include "../util.h"

#define MAX_RESULT_SIZE 128

/* Module name */
char module_name[] = "exec";

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "exec", "get_exec", 10.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "execbar", "get_execbar", 10.0, VARIABLE_BAR | ARGSTR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_exec(char *args)
{
        FILE *execp;
        char pipe[MAX_RESULT_SIZE];
        char *fgets_check;

        execp = popen(args, "r");
        if (execp != NULL) {
                fgets_check = fgets(pipe, MAX_RESULT_SIZE, execp);
                pclose(execp);
                if (fgets_check != NULL) {
                        chomp(pipe);
                        return m_strdup(pipe);
                }
        }

        return "n/a";
}

unsigned int get_execbar(char *args)
{
        FILE *execp;
        char pipe[MAX_RESULT_SIZE];
        char *fgets_check;

        execp = popen(args, "r");
        if (execp != NULL) {
                fgets_check = fgets(pipe, MAX_RESULT_SIZE, execp);
                pclose(execp);
                if (fgets_check != NULL)
                        return atoi(pipe);
        }

        return 0;
}

