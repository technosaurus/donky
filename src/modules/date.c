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
#include <time.h>

#include "../c99.h"
#include "../util.h"
#include "../mem.h"
#include "../module.h"

char module_name[] = "date_shet"; /* Up to 63 characters, any more and it will
                                     be truncated!  Doesn't matter though, just
                                     needs to be a some-what unique name. */

/* My function prototypes. */
char *get_date(char *args);

/**
 * @brief This is run on module initialization.
 */
void module_init(const struct module *mod)
{
        module_var_add(mod, "date", "get_date", 1.0, VARIABLE_STR);
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{
        
}

/**
 * @brief Get the current time in a custom format.
 *
 * @param args Argument string.
 *
 * @return Formatted time string
 */
char *get_date(char *args)
{        
        char ret_value[96];
        time_t t;
        struct tm *tmp;

        /* %c represents the recommended time display for the current locale. */
        if (args == NULL || is_all_spaces(args))
                args = "%c";

        t = time(NULL);
        tmp = localtime(&t);

        if (tmp == NULL)
                return "n/a";

        if (strftime(ret_value,
                     sizeof(ret_value) - sizeof(char),
                     args, tmp) == 0)
                return "n/a";

        return m_strdup(ret_value);
}
