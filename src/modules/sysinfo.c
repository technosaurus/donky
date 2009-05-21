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
#include <sys/sysinfo.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

char module_name[] = "sysinfo"; /* Up to 63 characters, any more and it will
                                   be truncated!  Doesn't matter though, just
                                   needs to be a some-what unique name. */

/* My function prototypes. */
char *get_uptime(char *args);
char *get_loadavg(char *args);
char *get_totalram(char *args);
char *get_freeram(char *args);
char *get_usedram(char *args);
char *get_sharedram(char *args);
char *get_bufferram(char *args);
char *get_totalswap(char *args);
char *get_freeswap(char *args);
char *get_usedswap(char *args);
char *get_procs(char *args);
char *get_totalhigh(char *args);
char *get_freehigh(char *args);
char *get_usedhigh(char *args);

/* Globals. */
char *ret = NULL;
struct sysinfo info;

/**
 * @brief This is run on module initialization.
 */
void module_init(struct module *mod)
{
        module_var_add(mod, "uptime", "get_uptime", 1.0, VARIABLE_STR);
        module_var_add(mod, "loadavg", "get_loadavg", 30.0, VARIABLE_STR);
        module_var_add(mod, "totalram", "get_totalram", 0.0, VARIABLE_STR);
        module_var_add(mod, "freeram", "get_freeram", 15.0, VARIABLE_STR);
        module_var_add(mod, "usedram", "get_usedram", 15.0, VARIABLE_STR);
        module_var_add(mod, "sharedram", "get_sharedram", 15.0, VARIABLE_STR);
        module_var_add(mod, "bufferram", "get_bufferram", 15.0, VARIABLE_STR);
        module_var_add(mod, "totalswap", "get_totalswap", 0.0, VARIABLE_STR);
        module_var_add(mod, "freeswap", "get_freeswap", 15.0, VARIABLE_STR);
        module_var_add(mod, "usedswap", "get_usedswap", 15.0, VARIABLE_STR);
        module_var_add(mod, "procs", "get_procs", 10.0, VARIABLE_STR);
        module_var_add(mod, "totalhigh", "get_totalhigh", 0.0, VARIABLE_STR);
        module_var_add(mod, "freehigh", "get_freehigh", 15.0, VARIABLE_STR);
        module_var_add(mod, "usedhigh", "get_usedhigh", 15.0, VARIABLE_STR);
}

/**
 * @brief This is run when the module is about to be unloaded.
 */
void module_destroy(void)
{

}

char *get_uptime(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";

        unsigned long cur_up = info.uptime;
        unsigned long days = cur_up / 86400;
        cur_up -= days * 86400;
        int hours = cur_up / 3600;
        cur_up -= hours * 3600;
        int mins = cur_up / 60;
        cur_up -= mins * 60;
        int secs = cur_up;

        ret = m_malloc(32 * sizeof(char));
        snprintf(ret, 32, "%ld day%s, %02d:%02d:%02d",
                 days, (days != 1) ? "s" : "",
                 hours, mins, secs);

        return ret;
}


char *get_loadavg(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";

        float load0, load1, load2;
        load0 = info.loads[0] / 65536.0;
        load1 = info.loads[1] / 65536.0;
        load2 = info.loads[2] / 65536.0;

        ret = m_malloc(32 * sizeof(char));
        snprintf(ret, 32, "%2.2f, %2.2f, %2.2f",
                 load0, load1, load2);

        return ret;
}

char *get_totalram(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.totalram * info.mem_unit));
}

char *get_freeram(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.freeram * info.mem_unit));
}

char *get_usedram(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger((info.totalram - info.freeram) * info.mem_unit));
}

char *get_sharedram(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.sharedram * info.mem_unit));
}

char *get_bufferram(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.bufferram * info.mem_unit));
}

char *get_totalswap(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.totalswap * info.mem_unit));
}

char *get_freeswap(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.freeswap * info.mem_unit));
}

char *get_usedswap(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger((info.totalswap - info.freeswap) * info.mem_unit));
}

char *get_procs(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";

        ret = m_malloc(8 * sizeof(char));
        snprintf(ret, 8, "%d", info.procs);
        return ret;
}

char *get_totalhigh(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.totalhigh * info.mem_unit));
}

char *get_freehigh(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger(info.freehigh * info.mem_unit));
}

char *get_usedhigh(char *args)
{
        if (sysinfo(&info) == -1)
                return "n/a";
        
        return m_freelater(bytes_to_bigger((info.totalhigh - info.freehigh) * info.mem_unit));
}
