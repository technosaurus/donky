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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "eeebl";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function prototypes */
char *get_eeeblper(char *args);
char *get_eeeblcur(char *args);
char *get_eeeblmax(char *args);
int get_eeeblbar(char *args);

void get_cur_bl(void);
void get_max_bl(void);

/* Globals */
char *cur_bl = NULL;
char *max_bl = NULL;

/* These run on module startup */
int module_init(void)
{
        module_var_add(module_name, "eeeblper", "get_eeeblper", 5.0, VARIABLE_STR);
        module_var_add(module_name, "eeeblcur", "get_eeeblcur", 5.0, VARIABLE_STR);
        module_var_add(module_name, "eeeblmax", "get_eeeblmax", 5.0, VARIABLE_STR);
        module_var_add(module_name, "eeeblbar", "get_eeeblbar", 5.0, VARIABLE_BAR);
        module_var_cron_add(module_name, "eeebl_cron", "eeebl_cron", 5.0);
        get_max_bl();
}

/* These run on module unload */
void module_destroy(void)
{
        freenullif(cur_bl);
        freenullif(max_bl);
}

/** 
 * @brief Updates cur_bl with the current remaining battery charge.
 */
void eeebl_cron(void)
{
        freenullif(cur_bl);
        get_cur_bl();
}

/** 
 * @brief Returns current backlight level in percentage format.
 * 
 * @param args Irrelevant.
 * 
 * @return String of current backlight level percentage.
 */
char *get_eeeblper(char *args)
{
        int charge;
        char *ret_eeeblper;

        if (cur_bl && max_bl) {
                charge = (atof(cur_bl) / atof(max_bl)) * 100;
                asprintf(&ret_eeeblper, "%d", charge);
                return m_freelater(ret_eeeblper);
        }

        return "n/a";
}

/** 
 * @brief Returns current backlight level.
 * 
 * @param args Irrelevant.
 * 
 * @return String of current backlight level.
 */
char *get_eeeblcur(char *args)
{
        return (cur_bl) ? cur_bl : "n/a";
}

/** 
 * @brief Returns maximum backlight level.
 * 
 * @param args Irrelevant.
 * 
 * @return String of maximum backlight level.
 */
char *get_eeeblmax(char *args)
{
        return (max_bl) ? max_bl : "n/a";
}

/** 
 * @brief Used in drawing a bar for current backlight level.
 * 
 * @param args Irrelevant.
 * 
 * @return Integer representing current backlight level percentage.
 */
int get_eeeblbar(char *args)
{
        if (cur_bl && max_bl)
                return (int)((atof(cur_bl) / atof(max_bl)) * 100);

        return 0;
}

/** 
 * @brief Updates cur_bl with the current backlight level from /sys.
 */
void get_cur_bl(void)
{
        char *path;
        FILE *cur_bl_file;
        size_t len;

        path = "/sys/devices/virtual/backlight/eeepc/brightness";
        cur_bl_file = fopen(path, "r");
        if (!cur_bl_file)
                return;

        len = 0;
        getline(&cur_bl, &len, cur_bl_file);
        fclose(cur_bl_file);
        if (len && cur_bl)
                chomp(cur_bl);
}

/** 
 * @brief Updates max_bl with the maximum backlight level from /sys. This only
 *        needs to be run once and never again, because it doesn't change.
 */
void get_max_bl(void)
{
        char *path;
        FILE *max_bl_file;
        size_t len;

        path = "/sys/devices/virtual/backlight/eeepc/max_brightness";
        max_bl_file = fopen(path, "r");
        if (!max_bl_file)
                return;

        len = 0;
        getline(&max_bl, &len, max_bl_file);
        fclose(max_bl_file);
        if (len && max_bl)
                chomp(max_bl);
}

