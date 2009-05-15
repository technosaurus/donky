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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "pcpuinfo";

/* My function prototypes */
char *get_pcpufreq(char *args);
char *get_pcpuname(char *args);
char *get_pcpucache(char *args);

/* Globals */
char *ret_pcpufreq = NULL;
char *ret_pcpuname = NULL;
char *ret_pcpucache = NULL;

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "pcpufreq", "get_pcpufreq", 1.0, VARIABLE_STR);
        module_var_add(mod, "pcpuname", "get_pcpuname", 0.0, VARIABLE_STR);
        module_var_add(mod, "pcpucache", "get_pcpucache", 0.0, VARIABLE_STR);
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

