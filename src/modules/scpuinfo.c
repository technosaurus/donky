/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

