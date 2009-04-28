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

#include "../mem.h"
#include "../module.h"

/* Module name */
char module_name[] = "scpuinfo";

/* Required function prototypes. */
void module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_scpufreq(char *args);

/* These run on module startup */
void module_init(void)
{
        module_var_add(module_name,
                       "scpufreq",
                       "get_scpufreq",
                       1.0,
                       VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_scpufreq(char *args)
{
        char *path = NULL;
        if ((asprintf(&path,
                      "/sys/devices/system/cpu/cpu%s/cpufreq/scaling_cur_freq",
                      (args) ? args : "0") == -1))
                return "n/a";

        FILE *freq_file = fopen(path, "r");
        free(path);
        if (!freq_file)
                return "n/a";

        char *freq = NULL;
        size_t len = 0;
        int read = getline(&freq, &len, freq_file);
        fclose(freq_file);
        if (!freq || (read == -1))
                return "n/a";

        char *ret = NULL;
        int check = asprintf(&ret, "%d", (atoi(freq) / 1000));
        free(freq);
        if (check == -1)
                return "n/a";

        return m_freelater(ret);
}

