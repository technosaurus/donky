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
#include "../mem.h"

/* Module name */
char module_name[] = "scpuinfo";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_scpufreq(char *args);

/* Globals */
char *ret_scpufreq = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "scpufreq", "get_scpufreq", 1.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_scpufreq(char *args)
{
        FILE *freq_file;
        
        char path[64];

        if (args == NULL)
                strncpy(path,
                        "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq\0",
                        (54 * sizeof(char)));
        else
                snprintf(path,
                        (63 * sizeof(char)),
                        "/sys/devices/system/cpu/cpu%s/cpufreq/scaling_cur_freq",
                        args);

        char freq[10];

        ret_scpufreq = NULL;

        freq_file = fopen(path, "r");
        if (freq_file == NULL) {
                ret_scpufreq = m_strdup("n/a");
                return ret_scpufreq;
        }

        if (fgets(freq, 10, freq_file) == NULL) {
                fclose(freq_file);
                ret_scpufreq = m_strdup("n/a");
                return ret_scpufreq;
        }

        ret_scpufreq = m_malloc(6 * sizeof(char));
        snprintf(ret_scpufreq, (5 * sizeof(char)), "%d", (atoi(freq) / 1000));
        
        fclose(freq_file);

        return ret_scpufreq;
}

