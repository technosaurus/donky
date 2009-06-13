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

#include "../mem.h"
#include "../module.h"
#include "../util.h"

#define DEFAULT_PATH "/sys/devices/system/cpu/cpu0/cpufreq/scaling_cur_freq"

/* Module name */
char module_name[] = "scpuinfo";

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "scpufreq", "get_scpufreq", 1.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_scpufreq(char *args)
{
        char *path;
        FILE *freq_file;
        char *fgets_check;
        char freq[16];
        int freq_int;

        if (args != NULL) {
                path = dstrdup("/sys/devices/system/cpu/cpu");
                stracat(&path, args);
                stracat(&path, "/cpufreq/scaling_cur_freq");
                freq_file = fopen(path, "r");
                free(path);
        } else {
                freq_file = fopen(DEFAULT_PATH, "r");
        }

        if (!freq_file)
                return "n/a";

        fgets_check = fgets(freq, sizeof(freq), freq_file);
        fclose(freq_file);
        if (fgets_check == NULL)
                return "n/a";

        freq_int = atoi(freq) / 1000;
        uint_to_str(freq, freq_int, sizeof(freq));

        return m_strdup(freq);
}

