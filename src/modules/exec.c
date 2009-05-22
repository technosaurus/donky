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

#include "../c99.h"
#include "../mem.h"
#include "../module.h"
#include "../util.h"

#define MAX_RESULT_SIZE 128

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
        char pipe[MAX_RESULT_SIZE];
        char *fgets_check;

        execp = popen(args, "r");
        if (execp != NULL) {
                fgets_check = fgets(pipe, MAX_RESULT_SIZE, execp);
                pclose(execp);
                if (fgets_check != NULL) {
                        chomp(pipe);
                        return m_strdup(pipe);
                }
        }

        return "n/a";
}

unsigned int get_execbar(char *args)
{
        FILE *execp;
        char pipe[MAX_RESULT_SIZE];
        char *fgets_check;

        execp = popen(args, "r");
        if (execp != NULL) {
                fgets_check = fgets(pipe, MAX_RESULT_SIZE, execp);
                pclose(execp);
                if (fgets_check != NULL)
                        return atoi(pipe);
        }

        return 0;
}

