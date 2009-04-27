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

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "pcpuinfo";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_pcpufreq(char *args);
char *get_pcpuname(char *args);
char *get_pcpucache(char *args);

/* Globals */
char *ret_pcpufreq = NULL;
char *ret_pcpuname = NULL;
char *ret_pcpucache = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "pcpufreq", "get_pcpufreq", 1.0, VARIABLE_STR);
        module_var_add(module_name, "pcpuname", "get_pcpuname", 0.0, VARIABLE_STR);
        module_var_add(module_name, "pcpucache", "get_pcpucache", 0.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_pcpufreq(char *args)
{
        FILE *pcpuinfo = fopen("/proc/cpuinfo", "r");

        ret_pcpufreq = NULL;

        if (pcpuinfo == NULL)
                return "Can't open /proc/cpuinfo";

        char str[16];

        char core;
        int found = 0;
        char *p;
        
        if (args == NULL)
                core = '0';
        else
                core = args[0];
        
        while (fgets(str, 16, pcpuinfo) != NULL) {
                if (strchr((str + 12), core) != NULL)
                        found = 1;
                if (found && (strncmp(str, "cpu MHz", 7) == 0)) {
                        ret_pcpufreq = m_strndup((str + 11), 4);

                        /* Null the decimal point if it's there. */
                        if (p = strchr((ret_pcpufreq + 3), '.'))
                                *p = '\0';
                        
                        break;
                }
        }

        fclose(pcpuinfo);

        if (ret_pcpufreq == NULL)
                return "CPU freq not found.";

        return ret_pcpufreq;
}

char *get_pcpuname(char *args)
{
        FILE *pcpuinfo = fopen("/proc/cpuinfo", "r");

        ret_pcpuname = NULL;

        if (pcpuinfo == NULL)
                return "Can't open /proc/cpuinfo";

        char str[64];

        char core;
        int found = 0;

        if (args == NULL)
                core = '0';
        else
                core = args[0];
        
        while (fgets(str, 64, pcpuinfo) != NULL) {
                if (strchr((str + 12), core) != NULL)
                        found = 1;
                if (found && (strncmp(str, "model name", 10) == 0)) {
                        ret_pcpuname = m_strdup(str + 13);
                        chomp(ret_pcpuname);
                        break;
                }
        }

        fclose(pcpuinfo);

        if (ret_pcpuname == NULL)
                return "CPU name not found.";

        return ret_pcpuname;
}

char *get_pcpucache(char *args)
{
        FILE *pcpuinfo = fopen("/proc/cpuinfo", "r");

        ret_pcpucache = NULL;

        if (pcpuinfo == NULL)
                return "Can't open /proc/cpuinfo";

        char str[16];

        char core;
        int found = 0;

        if (args == NULL)
                core = '0';
        else
                core = args[0];
        
        while (fgets(str, 16, pcpuinfo) != NULL) {
                if (strchr((str + 12), core) == NULL)
                        found = 1;
                if (found && (strncmp(str, "cache size", 10) == 0)) {
                        ret_pcpucache = m_strndup((str + 13), 4);
                        trim_t(ret_pcpucache);
                        break;
                }
        }

        fclose(pcpuinfo);

        if (ret_pcpucache == NULL)
                return "CPU not found.";

        return ret_pcpucache;
}

