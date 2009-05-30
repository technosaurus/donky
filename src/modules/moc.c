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
char module_name[] = "moc";

/* Globals */
static char home[DMAXPATHLEN];

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "moc", "get_moc", 10.0, VARIABLE_STR);
        snprintf(home, sizeof(home), "%s", getenv("HOME"));
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_moc(char *args)
{
        FILE *mocp;
        char pipe[160];
        size_t size;
        char *mocp_line;

        size = strlen(args) + strlen(home) + (sizeof(char) * 19);
        mocp_line = malloc(size);

        snprintf(mocp_line,
                 size,
                 "HOME=\"%s\" mocp -Q \"%s\"",
                 home, args);

        mocp = popen(mocp_line, "r");
        free(mocp_line);
        if (mocp == NULL)
                return "Could not query moc.";

        if (fgets(pipe, sizeof(pipe), mocp) == NULL) {
                pclose(mocp);
                return "No music playing.";
        }

        pclose(mocp);

        return m_strdup(chomp(pipe));
}

