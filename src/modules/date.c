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
#include <time.h>

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
