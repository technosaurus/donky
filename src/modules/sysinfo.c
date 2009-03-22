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
#include <math.h>

#include "develop.h"
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
char *ret_uptime = NULL;
char *ret_loadavg = NULL;
char *ret_totalram = NULL;
char *ret_freeram = NULL;
char *ret_usedram = NULL;
char *ret_sharedram = NULL;
char *ret_bufferram = NULL;
char *ret_totalswap = NULL;
char *ret_freeswap = NULL;
char *ret_usedswap = NULL;
char *ret_procs = NULL;
char *ret_totalhigh = NULL;
char *ret_freehigh = NULL;
char *ret_usedhigh = NULL;

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
        /* Free everything. */
        freeif(ret_uptime);
        freeif(ret_loadavg);
        freeif(ret_totalram);
        freeif(ret_freeram);
        freeif(ret_usedram);
        freeif(ret_sharedram);
        freeif(ret_bufferram);
        freeif(ret_totalswap);
        freeif(ret_freeswap);
        freeif(ret_usedswap);
        freeif(ret_procs);
        freeif(ret_totalhigh);
        freeif(ret_freehigh);
        freeif(ret_usedhigh);
}

/**
 * @brief Convert raw bytes into formatted values.
 *
 * @param bytes Total bytes
 *
 * @return Formatted string
 */
char *bytes_to_bigger(unsigned long bytes)
{
        char str[32];
        char label[4];
        double tera = pow(1024, 4);
        double giga = pow(1024, 3);
        double mega = pow(1024, 2);
        double kilo = 1024;
        double recalc = 0.0;

        if (bytes >= tera) {
                recalc = bytes / tera;
                strncpy(label, "TiB", sizeof(label) - 1);
        } else if (bytes >= giga) {
                recalc = bytes / giga;
                strncpy(label, "GiB", sizeof(label) - 1);
        } else if (bytes >= mega) {
                recalc = bytes / mega;
                strncpy(label, "MiB", sizeof(label) - 1);
        } else if (bytes >= kilo) {
                recalc = bytes / kilo;
                strncpy(label, "KiB", sizeof(label) - 1);
        } else {
                recalc = bytes;
                strncpy(label, "B", sizeof(label) - 1);
        }

        label[3] = '\0';
        snprintf(str, sizeof(str),
                 "%.2f%s", recalc, label);

        return strdup(str);
}

char *get_uptime(char *args)
{
        freeif(ret_uptime);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_uptime = strdup("n/a");
                return ret_uptime;
        }

        unsigned long cur_up = info.uptime;
        unsigned long days = cur_up / 86400;
        cur_up -= days * 86400;
        int hours = cur_up / 3600;
        cur_up -= hours * 3600;
        int mins = cur_up / 60;
        cur_up -= mins * 60;
        int secs = cur_up;

        ret_uptime = malloc(32 * sizeof(char));
        snprintf(ret_uptime, 32, "%ld day%s, %02d:%02d:%02d",
                 days, (days != 1) ? "s" : "",
                 hours, mins, secs);

        return ret_uptime;
}


char *get_loadavg(char *args)
{
        freeif(ret_loadavg);

        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_loadavg = strdup("n/a");
                return ret_loadavg;
        }

        float load0, load1, load2;
        load0 = info.loads[0] / 65536.0;
        load1 = info.loads[1] / 65536.0;
        load2 = info.loads[2] / 65536.0;

        ret_loadavg = malloc(32 * sizeof(char));
        snprintf(ret_loadavg, 32, "%2.2f, %2.2f, %2.2f",
                 load0, load1, load2);

        return ret_loadavg;
}

char *get_totalram(char *args)
{
        freeif(ret_totalram);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_totalram = strdup("n/a");
                return ret_totalram;
        }
        
        ret_totalram = bytes_to_bigger(info.totalram * info.mem_unit);
        return ret_totalram;
}

char *get_freeram(char *args)
{
        freeif(ret_freeram);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_freeram = strdup("n/a");
                return ret_freeram;
        }
        
        ret_freeram = bytes_to_bigger(info.freeram * info.mem_unit);
        return ret_freeram;
}

char *get_usedram(char *args)
{
        freeif(ret_usedram);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_usedram = strdup("n/a");
                return ret_usedram;
        }
        
        ret_usedram = bytes_to_bigger((info.totalram - info.freeram) *
                                       info.mem_unit);
        return ret_usedram;
}

char *get_sharedram(char *args)
{
        freeif(ret_sharedram);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_sharedram = strdup("n/a");
                return ret_sharedram;
        }
        
        ret_sharedram = bytes_to_bigger(info.sharedram * info.mem_unit);
        return ret_sharedram;
}

char *get_bufferram(char *args)
{
        freeif(ret_bufferram);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_bufferram = strdup("n/a");
                return ret_bufferram;
        }
        
        ret_bufferram = bytes_to_bigger(info.bufferram * info.mem_unit);
        return ret_bufferram;
}

char *get_totalswap(char *args)
{
        freeif(ret_totalswap);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_totalswap = strdup("n/a");
                return ret_totalswap;
        }
        
        ret_totalswap = bytes_to_bigger(info.totalswap * info.mem_unit);
        return ret_totalswap;
}

char *get_freeswap(char *args)
{
        freeif(ret_freeswap);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_freeswap = strdup("n/a");
                return ret_freeswap;
        }
        
        ret_freeswap = bytes_to_bigger(info.freeswap * info.mem_unit);
        return ret_freeswap;
}

char *get_usedswap(char *args)
{
        freeif(ret_usedswap);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_usedswap = strdup("n/a");
                return ret_usedswap;
        }
        
        ret_usedswap = bytes_to_bigger((info.totalswap - info.freeswap) * info.mem_unit);
        return ret_usedswap;
}

char *get_procs(char *args)
{
        freeif(ret_procs);

        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_procs = strdup("n/a");
                return ret_procs;
        }

        ret_procs = malloc(8 * sizeof(char));
        snprintf(ret_procs, 8, "%d", info.procs);
        return ret_procs;
}

char *get_totalhigh(char *args)
{
        freeif(ret_totalhigh);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_totalhigh = strdup("n/a");
                return ret_totalhigh;
        }
        
        ret_totalhigh = bytes_to_bigger(info.totalhigh * info.mem_unit);
        return ret_totalhigh;
}

char *get_freehigh(char *args)
{
        freeif(ret_freehigh);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_freehigh = strdup("n/a");
                return ret_freehigh;
        }
        
        ret_freehigh = bytes_to_bigger(info.freehigh * info.mem_unit);
        return ret_freehigh;
}

char *get_usedhigh(char *args)
{
        freeif(ret_usedhigh);
        
        struct sysinfo info;
        if (sysinfo(&info) == -1) {
                ret_usedhigh = strdup("n/a");
                return ret_usedhigh;
        }
        
        ret_usedhigh = bytes_to_bigger((info.totalhigh - info.freehigh) * info.mem_unit);
        return ret_usedhigh;
}
