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
#include "../util.h"

/* Module name */
char module_name[] = "cpu_info";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_cpufreq(char *args);
char *get_cpuname(char *args);
char *get_cpucache(char *args);

/* Globals */
char *ret_cpufreq = NULL;
char *ret_cpuname = NULL;
char *ret_cpucache = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "cpufreq", "get_cpufreq", 1.0, VARIABLE_STR);
        module_var_add(module_name, "cpuname", "get_cpuname", 0.0, VARIABLE_STR);
        module_var_add(module_name, "cpucache", "get_cpucache", 0.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{
        freeif(ret_cpufreq);
        freeif(ret_cpuname);
        freeif(ret_cpucache);
}

char *get_cpufreq(char *args)
{
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        
        int n = 16;
        char *str = malloc(n * sizeof(char));

        char *core = NULL;
        int found = 0;

        freeif(ret_cpufreq);
        ret_cpufreq = NULL;

        if (args == NULL)
                core = d_strncpy("0", 1);
        else
                core = args;
        
        while (fgets(str, n, cpuinfo) != NULL) {
                if (strncmp((str + 12), core, 1) == 0)
                        found = 1;
                if (found && (strncmp(str, "cpu MHz", 7) == 0)) {
                        /* Copy up until the decimal point,
                         * possibly including it. */
                        ret_cpufreq = d_strncpy((str + 11), 4);

                        /* Remove the new line, else it prints junk */
                        chomp(ret_cpufreq);

                        /* Remove decimal point if it's there. */
                        if (ret_cpufreq[3] == '.')
                                ret_cpufreq[3] = '\0';
                        
                        break;
                }
        }

        free(str);
        fclose(cpuinfo);

        if (ret_cpufreq == NULL)
                ret_cpufreq = d_strncpy("CPU not found.", 14);

        return ret_cpufreq;
}

char *get_cpuname(char *args)
{
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        
        int n = 64;
        char *str = malloc(n * sizeof(char));

        char *core = NULL;
        int found = 0;

        freeif(ret_cpuname);
        ret_cpuname = NULL;

        if (args == NULL)
                core = d_strncpy("0", 1);
        else
                core = args;
        
        while (fgets(str, n, cpuinfo) != NULL) {
                if (strncmp((str + 12), core, 1) == 0)
                        found = 1;
                if (found && (strncmp(str, "model name", 10) == 0)) {
                        ret_cpuname = d_strncpy((str + 13), 64);
                        chomp(ret_cpuname);
                        break;
                }
        }

        free(str);
        fclose(cpuinfo);

        if (ret_cpuname == NULL)
                ret_cpuname = d_strncpy("CPU not found.", 14);

        return ret_cpuname;
}

char *get_cpucache(char *args)
{
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        
        int n = 16;
        char *str = malloc(n * sizeof(char));

        char *core = NULL;
        int found = 0;

        freeif(ret_cpucache);
        ret_cpucache = NULL;

        if (args == NULL)
                core = d_strncpy("0", 1);
        else
                core = args;
        
        while (fgets(str, n, cpuinfo) != NULL) {
                if (strncmp((str + 12), core, 1) == 0)
                        found = 1;
                if (found && (strncmp(str, "cache size", 10) == 0)) {
                        ret_cpucache = d_strncpy((str + 13), 4);
                        trim_t(ret_cpucache);
                        break;
                }
        }

        free(str);
        fclose(cpuinfo);

        if (ret_cpucache == NULL)
                ret_cpucache = d_strncpy("CPU not found.", 14);

        return ret_cpucache;
}

