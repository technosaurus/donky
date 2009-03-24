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

#include "develop.h"
#include "../mem.h"

/* Module name */
char module_name[] = "eeepc";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_eee_backlight(char *args);
void get_backlight_max(void);

/* Globals */
char *ret_backlight = NULL;
FILE *backlight_max;
char max[4];

/* These run on module startup */
int module_init(void)
{
        get_backlight_max();
        module_var_add(module_name, "eeebl", "get_eee_backlight", 5.0, VARIABLE_STR);
}

/* These run on module unload */
void module_destroy(void)
{

}

char *get_eee_backlight(char *args)
{
        ret_backlight = NULL;
        
        if (max[0] == '\0') {
                ret_backlight = m_strdup("n/a");
                return ret_backlight;
        }

        FILE *backlight_now;

        char now[4];

        int percent;

        backlight_now = fopen("/sys/devices/virtual/backlight/eeepc/brightness", "r");
        if (backlight_now == NULL) {
                ret_backlight = m_strdup("n/a");
                return ret_backlight;
        }

        if (fgets(now, 16, backlight_now) == NULL) {
                fclose(backlight_now);
                ret_backlight = m_strdup("n/a");
                return ret_backlight;
        }

        percent = ((atof(now) / atof(max)) * 100);
        
        ret_backlight = m_malloc(5 * sizeof(char));
        snprintf(ret_backlight, (4 * sizeof(char)), "%d", percent);
        
        fclose(backlight_now);

        return ret_backlight;
}

void get_backlight_max(void)
{
        backlight_max = fopen("/sys/devices/virtual/backlight/eeepc/max_brightness", "r");
        if (backlight_max == NULL) {
                max[0] = '\0';
                return;
        }
        
        if (fgets(max, sizeof(max), backlight_max) == NULL) {
                fclose(backlight_max);
                max[0] = '\0';
                return;
        }

        fclose(backlight_max);
}
