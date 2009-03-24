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

#include "develop.h"
#include "../mem.h"

/* Module name */
char module_name[] = "battery";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_battery(char *args);
void get_full_charge(void);

/* Globals */
char *ret_battery = NULL;
char full[16];

/* These run on module startup */
int module_init(void)
{
        get_full_charge();
        module_var_add(module_name, "battery", "get_battery", 30.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_battery(char *args)
{
        ret_battery = NULL;

        if (full[0] == '\0') {
                ret_battery = m_strdup("n/a");
                return ret_battery;
        }

        char path[64];

        if (args == NULL)
                strncpy(path,
                        "/sys/class/power_supply/BAT0/charge_now\0",
                        (40 * sizeof(char)));
        else
                snprintf(path,
                        (63 * sizeof(char)),
                        "/sys/class/power_supply/BAT%s/charge_now",
                        args);

        FILE *charge_now;

        char now[16];

        int charge;

        charge_now = fopen(path, "r");
        if (charge_now == NULL) {
                ret_battery = m_strdup("n/a");
                return ret_battery;
        }

        if (fgets(now, 16, charge_now) == NULL) {
                fclose(charge_now);
                ret_battery = m_strdup("n/a");
                return ret_battery;
        }

        charge = ((atof(now) / atof(full)) * 100);
        
        ret_battery = m_malloc(5 * sizeof(char));
        snprintf(ret_battery, (4 * sizeof(char)), "%d", charge);
        
        fclose(charge_now);

        return ret_battery;
}

/** 
 * @brief Get the full charge capacity of the battery.
 *        This only needs to be checked once at module load,
 *        and never again.
 */
void get_full_charge(void)
{
        FILE *charge_full;

        charge_full = fopen("/sys/class/power_supply/BAT0/charge_full", "r");
        if (charge_full == NULL) {
                full[0] = '\0';
                return;
        }
        
        if (fgets(full, 16, charge_full) == NULL) {
                fclose(charge_full);
                full[0] = '\0';
                return;
        }

        fclose(charge_full);
}

