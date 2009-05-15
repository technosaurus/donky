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
#include <sys/param.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "moc";

/* My function prototypes */
char *get_moc(char *args);

/* Globals */
char home[MAXPATHLEN + 1];

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "moc", "get_moc", 10.0, VARIABLE_STR);
        snprintf(home, MAXPATHLEN, "%s", getenv("HOME"));
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_moc(char *args)
{
        FILE *mocp;
        
        char pipe[160];
        
        size_t size = strlen(args) + strlen(home) + (sizeof(char) * 19);
        char mocp_line[size];

        snprintf(mocp_line,
                 size,
                 "HOME=\"%s\" mocp -Q \"%s\"",
                 home, args);

        mocp = popen(mocp_line, "r");
        if (mocp == NULL)
                return "Could not query moc.";

        if (fgets(pipe, 160, mocp) == NULL) {
                pclose(mocp);
                return "No music playing.";
        }

        pclose(mocp);

        return m_strdup(chomp(pipe));
}

