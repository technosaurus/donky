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

/* Module name */
char module_name[] = "scpuinfo";

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "scpufreq", "get_scpufreq", 1.0, VARIABLE_STR | ARGSTR);
}

/* These run on module unload */
void module_destroy(void)
{

}

#define CPU_PRE  "/sys/devices/system/cpu/cpu"
#define CPU_POST "/cpufreq/scaling_cur_freq"
char *get_scpufreq(char *args)
{
        char *path;
        FILE *freq_file;
        char *fgets_check;
        char freq[16];

        asprintf(&path, CPU_PRE "%s" CPU_POST, (args != NULL) ? args : "0");
        freq_file = fopen(path, "r");
        free(path);
        if (freq_file == NULL)
                return "n/a";

        fgets_check = fgets(freq, sizeof(freq), freq_file);
        fclose(freq_file);
        if (fgets_check == NULL)
                return "n/a";

        snprintf(freq, sizeof(freq), "%d", atoi(freq) / 1000);

        return m_strdup(freq);
}

