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
#include "../util.h"

/* Module name */
char module_name[] = "pcpuinfo";

/* Globals */
static char *ret_pcpufreq = NULL;
static char *ret_pcpuname = NULL;
static char *ret_pcpucache = NULL;

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "pcpufreq", "get_pcpufreq", 1.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "pcpuname", "get_pcpuname", 0.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "pcpucache", "get_pcpucache", 0.0, VARIABLE_STR | ARGSTR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_pcpufreq(char *args)
{
        FILE *pcpuinfo = fopen("/proc/cpuinfo", "r");
        char str[16];
        char core;
        int found = 0;
        char *p;
 
        ret_pcpufreq = NULL;

        if (pcpuinfo == NULL)
                return "Can't open /proc/cpuinfo";
       
        if (args == NULL)
                core = '0';
        else
                core = args[0];
        
        while (fgets(str, 16, pcpuinfo) != NULL) {
                if (strchr((str + 12), core) != NULL)
                        found = 1;
                if (found && (strncmp(str, "cpu MHz", 7) == 0)) {
                        ret_pcpufreq = m_strdup(str + 11);

                        /* Null the decimal point if it's there. */
                        if ((p = strchr((ret_pcpufreq + 3), '.')))
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
        char str[64];
        char core;
        int found = 0;

        ret_pcpuname = NULL;

        if (pcpuinfo == NULL)
                return "Can't open /proc/cpuinfo";

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
        char str[16];
        char core;
        int found = 0;

        ret_pcpucache = NULL;

        if (pcpuinfo == NULL)
                return "Can't open /proc/cpuinfo";

        if (args == NULL)
                core = '0';
        else
                core = args[0];

        while (fgets(str, 16, pcpuinfo) != NULL) {
                if (strchr((str + 12), core) == NULL)
                        found = 1;
                if (found && (strncmp(str, "cache size", 10) == 0)) {
                        ret_pcpucache = m_strdup(str + 13);
                        trim_t(ret_pcpucache);
                        break;
                }
        }

        fclose(pcpuinfo);

        if (ret_pcpucache == NULL)
                return "CPU not found.";

        return ret_pcpucache;
}

