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
#include "../util.h"

/* Module name */
char module_name[] = "eeebl";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_eeeblp(char *args);
char *get_eeeblc(char *args);
char *get_eeeblm(char *args);
int get_eeeblb(char *args);

void get_cur_bl(void);
void get_max_bl(void);

/* Globals */
char *cur_bl = NULL;
char *max_bl = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "eeeblp", "get_eeeblp", 5.0, VARIABLE_STR);
        module_var_add(module_name, "eeeblc", "get_eeeblc", 5.0, VARIABLE_STR);
        module_var_add(module_name, "eeeblm", "get_eeeblm", 5.0, VARIABLE_STR);

        module_var_add(module_name, "eeeblb", "get_eeeblb", 5.0, VARIABLE_BAR);

        module_var_cron_add(module_name,
                            "eeebl_cron",
                            "eeebl_cron",
                            5.0,
                            VARIABLE_CRON);

        get_max_bl();
}

/* These run on module unload */
void module_destroy(void)
{
        freeif(cur_bl);
        freeif(max_bl);
}

/** 
 * @brief Updates cur_bl with the current remaining battery charge.
 */
void eeebl_cron(void)
{
        if (cur_bl) {
                free(cur_bl);
                cur_bl = NULL;
        }

        get_cur_bl();
}

/** 
 * @brief Returns current backlight level in percentage format.
 * 
 * @param args Irrelevant.
 * 
 * @return String of current backlight level percentage.
 */
char *get_eeeblp(char *args)
{
        if ((cur_bl == NULL) || (max_bl == NULL))
                return m_strdup("n/a");

        int charge = (atof(cur_bl) / atof(max_bl)) * 100;

        char ret_eeeblp[8];
        snprintf(ret_eeeblp,
                 sizeof(ret_eeeblp) - sizeof(char),
                 "%d",
                 charge);

        return m_strdup(ret_eeeblp);
}

/** 
 * @brief Returns current backlight level.
 * 
 * @param args Irrelevant.
 * 
 * @return String of current backlight level.
 */
char *get_eeeblc(char *args)
{
        if (cur_bl == NULL)
                return m_strdup("n/a");
        else
                return m_strdup(cur_bl);
}

/** 
 * @brief Returns maximum backlight level.
 * 
 * @param args Irrelevant.
 * 
 * @return String of maximum backlight level.
 */
char *get_eeeblm(char *args)
{
        if (max_bl == NULL)
                return m_strdup("n/a");
        else
                return m_strdup(max_bl);
}

/** 
 * @brief Used in drawing a bar for current backlight level.
 * 
 * @param args Irrelevant.
 * 
 * @return Integer representing current backlight level percentage.
 */
int get_eeeblb(char *args)
{
        if ((cur_bl == NULL) || (max_bl == NULL))
                return 0;

        int charge = (atof(cur_bl) / atof(max_bl)) * 100;

        return charge;
}

/** 
 * @brief Updates cur_bl with the current backlight level from /sys.
 */
void get_cur_bl(void)
{
        char *path = "/sys/devices/virtual/backlight/eeepc/brightness";

        FILE *cur_bl_file = fopen(path, "r");
        if (cur_bl_file == NULL) {
                cur_bl = NULL;
                return;
        }

        char cur[16];
        if (fgets(cur, 16, cur_bl_file) == NULL) {
                fclose(cur_bl_file);
                cur_bl = NULL;
                return;
        }

        fclose(cur_bl_file);

        cur_bl = d_strcpy(chomp(cur));
}

/** 
 * @brief Updates max_bl with the maximum backlight level from /sys. This only
 *        needs to be run once and never again, because it doesn't change.
 */
void get_max_bl(void)
{
        char *path = "/sys/devices/virtual/backlight/eeepc/max_brightness";

        FILE *max_bl_file;
        max_bl_file = fopen(path, "r");
        if (max_bl_file == NULL) {
                cur_bl = NULL;
                return;
        }

        char max[16];
        if (fgets(max, 16, max_bl_file) == NULL) {
                fclose(max_bl_file);
                cur_bl = NULL;
                return;
        }

        fclose(max_bl_file);

        max_bl = d_strcpy(chomp(max));
}

