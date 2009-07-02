/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

