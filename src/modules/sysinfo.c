/*
 * This file is part of donky.
 *
 * donky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * donky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "develop.h"
#include "../mem.h"
#include "../util.h"

char module_name[] = "sysinfo"; /* Up to 63 characters, any more and it will
                                   be truncated!  Doesn't matter though, just
                                   needs to be a some-what unique name. */

/* Required function prototypes. */
void module_init(void);
void module_destroy(void);

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
void module_init(void)
{
        module_var_add(module_name, "uptime", "get_uptime", 1.0, VARIABLE_STR);
        module_var_add(module_name, "loadavg", "get_loadavg", 30.0, VARIABLE_STR);
        module_var_add(module_name, "totalram", "get_totalram", 0.0, VARIABLE_STR);
        module_var_add(module_name, "freeram", "get_freeram", 15.0, VARIABLE_STR);
        module_var_add(module_name, "usedram", "get_usedram", 15.0, VARIABLE_STR);
        module_var_add(module_name, "sharedram", "get_sharedram", 15.0, VARIABLE_STR);
        module_var_add(module_name, "bufferram", "get_bufferram", 15.0, VARIABLE_STR);
        module_var_add(module_name, "totalswap", "get_totalswap", 0.0, VARIABLE_STR);
        module_var_add(module_name, "freeswap", "get_freeswap", 15.0, VARIABLE_STR);
        module_var_add(module_name, "usedswap", "get_usedswap", 15.0, VARIABLE_STR);
        module_var_add(module_name, "procs", "get_procs", 10.0, VARIABLE_STR);
        module_var_add(module_name, "totalhigh", "get_totalhigh", 0.0, VARIABLE_STR);
        module_var_add(module_name, "freehigh", "get_freehigh", 15.0, VARIABLE_STR);
        module_var_add(module_name, "usedhigh", "get_usedhigh", 15.0, VARIABLE_STR);
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
                return m_strdup("n/a");

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
                return m_strdup("n/a");

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
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.totalram * info.mem_unit));
}

char *get_freeram(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.freeram * info.mem_unit));
}

char *get_usedram(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger((info.totalram - info.freeram) * info.mem_unit));
}

char *get_sharedram(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.sharedram * info.mem_unit));
}

char *get_bufferram(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.bufferram * info.mem_unit));
}

char *get_totalswap(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.totalswap * info.mem_unit));
}

char *get_freeswap(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.freeswap * info.mem_unit));
}

char *get_usedswap(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger((info.totalswap - info.freeswap) * info.mem_unit));
}

char *get_procs(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");

        ret = m_malloc(8 * sizeof(char));
        snprintf(ret, 8, "%d", info.procs);
        return ret;
}

char *get_totalhigh(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.totalhigh * info.mem_unit));
}

char *get_freehigh(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger(info.freehigh * info.mem_unit));
}

char *get_usedhigh(char *args)
{
        if (sysinfo(&info) == -1)
                return m_strdup("n/a");
        
        return m_freelater(bytes_to_bigger((info.totalhigh - info.freehigh) * info.mem_unit));
}
