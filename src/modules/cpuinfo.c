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

#include "develop.h"
#include "../util.h"

/* Module name */
char module_name[] = "cpu_info";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_cpufreq(char *args);
//char *get_cpuname(char *args);
//char *get_cpucache(char *args);

/* Globals */
char *ret_cpufreq = NULL;
//char *ret_cpuname = NULL;
//char *ret_cpucache = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "cpufreq", "get_cpufreq", 1.0, VARIABLE_STR);
        //module_var_add(module_name, "cpuname", "get_cpuname", 0.0, VARIABLE_STR);
        //module_var_add(module_name, "cpucache", "get_cpucache", 0.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{
        freeif(ret_cpufreq);
        //freeif(ret_cpuname);
        //freeif(ret_cpucache);
}

char *get_cpufreq(char *args)
{
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        
        char str[16];

        char core;
        int found = 0;
        char *p = NULL;
        
        freeif(ret_cpufreq);
        ret_cpufreq = NULL;

        if (args == NULL)
                core = '0';
        else
                core = args[0];
        
        while (fgets(str, 16, cpuinfo) != NULL) {
                if (strchr((str + 12), core) != NULL)
                        found = 1;
                if (found && (strncmp(str, "cpu MHz", 7) == 0)) {
                        ret_cpufreq = d_strncpy((str + 11), 4);

                        /* Put a null at the decimal point. */
                        if (p = strchr((ret_cpufreq + 3), '.'))
                                *p = '\0';
                        
                        break;
                }
        }

        //freeif(p);
        fclose(cpuinfo);

        if (ret_cpufreq == NULL)
                ret_cpufreq = d_strcpy("CPU freq not found.");

        return ret_cpufreq;
}
/*
char *get_cpuname(char *args)
{
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        
        char str[64];

        char *core = NULL;
        int found = 0;

        freeif(ret_cpuname);
        ret_cpuname = NULL;

        if (args == NULL)
                core = d_strncpy("0");
        else
                core = args;
        
        while (fgets(str, 64, cpuinfo) != NULL) {
                if (strncmp((str + 12), core, 1) == 0)
                        found = 1;
                if (found && (strncmp(str, "model name", 10) == 0)) {
                        ret_cpuname = d_strncpy(str + 13);
                        chomp(ret_cpuname);
                        break;
                }
        }

        free(str);
        fclose(cpuinfo);

        if (ret_cpuname == NULL)
                ret_cpuname = d_strncpy("CPU name not found.");

        return ret_cpuname;
}

char *get_cpucache(char *args)
{
        FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
        
        char str[16];
        char core;
        int found = 0;

        freeif(ret_cpucache);
        ret_cpucache = NULL;

        if (args == NULL)
                core = d_strncpy("0");
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
*/
