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

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "exec";

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "exec", "get_exec", 10.0, VARIABLE_STR);
        module_var_add(mod, "execbar", "get_execbar", 10.0, VARIABLE_BAR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_exec(char *args)
{
        FILE *execp;
        char *pipe;
        size_t len;

        execp = popen(args, "r");
        if (!execp)
                return "n/a";

        pipe = NULL;
        len = 0;
        getline(&pipe, &len, execp);
        pclose(execp);
        if (len && pipe)
                return m_freelater(chomp(pipe));

        return "n/a";
}

unsigned int get_execbar(char *args)
{
        FILE *execp;
        char *pipe;
        size_t len;
        int value;

        execp = popen(args, "r");
        if (!execp)
                return 0;

        pipe = NULL;
        len = 0;
        getline(&pipe, &len, execp);
        pclose(execp);
        if (len && pipe) {
                value = atoi(pipe);
                free(pipe);
                return value;
        }

        return 0;
}

