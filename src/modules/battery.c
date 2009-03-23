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

//#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
//#include <string.h>

#include "develop.h"
#include "../util.h"

/* Module name */
char module_name[] = "battery";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_battery(char *args);

/* Globals */
char *ret_battery = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "battery", "get_battery", 30.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{
        freeif(ret_battery);
}

char *get_battery(char *args)
{
        FILE *charge_now;
        FILE *charge_full;

        char now[16];
        char full[16];

        int charge;

        freeif(ret_battery);
        ret_battery = NULL;

        charge_now = fopen("/sys/class/power_supply/BAT0/charge_now", "r");
        if (charge_now == NULL) {
                ret_battery = d_strcpy("n/a");
                return ret_battery;
        }

        charge_full = fopen("/sys/class/power_supply/BAT0/charge_full", "r");
        if (charge_full == NULL) {
                fclose(charge_now);
                ret_battery = d_strcpy("n/a");
                return ret_battery;
        }

        if (fgets(now, 16, charge_now) == NULL) {
                fclose(charge_now);
                fclose(charge_full);
                ret_battery = d_strcpy("n/a");
                return ret_battery;
        }

        if (fgets(full, 16, charge_full) == NULL) {
                fclose(charge_now);
                fclose(charge_full);
                ret_battery = d_strcpy("n/a");
                return ret_battery;
        }

        charge = ((atof(now) / atof(full)) * 100);
        
        ret_battery = malloc(4 * sizeof(char));
        snprintf(ret_battery, (3 * sizeof(char)), "%d", charge);

        fclose(charge_now);
        fclose(charge_full);

        return ret_battery;
}

