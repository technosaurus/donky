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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../c99.h"
#include "../mem.h"
#include "../module.h"

/* Module name */
char module_name[] = "scpuinfo";

/* My function prototypes */
char *get_scpufreq(char *args);

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod,
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

