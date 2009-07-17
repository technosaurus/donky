/**
 * The CC0 1.0 Universal is applied to this work.
 *
 * To the extent possible under law, Matt Hayes and Jake LeMaster have
 * waived all copyright and related or neighboring rights to donky.
 * This work is published from the United States.
 *
 * Please see the copy of the CC0 included with this program for complete
 * information including limitations and disclaimers. If no such copy
 * exists, see <http://creativecommons.org/publicdomain/zero/1.0/legalcode>.
 */

#include <time.h>

#include "../util.h"
#include "../mem.h"
#include "../module.h"

char module_name[] = "date_shet"; /* Up to 63 characters, any more and it will
                                     be truncated!  Doesn't matter though, just
                                     needs to be a some-what unique name. */

/**
 * @brief This is run on module initialization.
 */
void module_init(const struct module *mod)
{
        module_var_add(mod, "date", "get_date", 1.0, VARIABLE_STR | ARGSTR);
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
