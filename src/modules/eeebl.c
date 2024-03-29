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

#include <stdio.h>
#include <stdlib.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "eeebl";

/* My function prototypes */
static void get_cur_bl(void);
static void get_max_bl(void);

/* Globals */
static char *cur_bl = NULL;
static char *max_bl = NULL;

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "eeeblper", "get_eeeblper", 5.0, VARIABLE_STR);
        module_var_add(mod, "eeeblcur", "get_eeeblcur", 5.0, VARIABLE_STR);
        module_var_add(mod, "eeeblmax", "get_eeeblmax", 5.0, VARIABLE_STR);
        module_var_add(mod, "eeeblbar", "get_eeeblbar", 5.0, VARIABLE_BAR);
        module_var_add(mod, "eeebl_cron", "eeebl_cron", 5.0, VARIABLE_CRON);
}

/* These run on module unload */
void module_destroy(void)
{
        freenull(cur_bl);
        freenull(max_bl);
}

/** 
 * @brief Updates cur_bl with the current remaining battery charge.
 */
void eeebl_cron(void)
{
        if (max_bl == NULL)
                get_max_bl();

        free(cur_bl);
        get_cur_bl();
}

/** 
 * @brief Returns current backlight level in percentage format.
 * 
 * @param args Irrelevant.
 * 
 * @return String of current backlight level percentage.
 */
char *get_eeeblper(void)
{
        int percentage;
        char ret_per[4];

        if (cur_bl && max_bl) {
                percentage = (atof(cur_bl) / atof(max_bl)) * 100;
                snprintf(ret_per, sizeof(ret_per), "%d", percentage);

                return m_strdup(ret_per);
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
char *get_eeeblcur(void)
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
char *get_eeeblmax(void)
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
unsigned int get_eeeblbar(void)
{
        if (cur_bl && max_bl)
                return (int)((atof(cur_bl) / atof(max_bl)) * 100);

        return 0;
}

/**
 * @brief Updates cur_bl with the current backlight level from /sys.
 */
static void get_cur_bl(void)
{
        char *path;
        FILE *cur_bl_file;
        char buf[4];

        path = "/sys/devices/virtual/backlight/eeepc/brightness";
        cur_bl_file = fopen(path, "r");
        if (!cur_bl_file) {
                cur_bl = NULL;
                return;
        }

        cur_bl = fgets(buf, sizeof(buf), cur_bl_file);
        fclose(cur_bl_file);
        if (cur_bl != NULL)
                chomp(cur_bl);
}

/** 
 * @brief Updates max_bl with the maximum backlight level from /sys. This only
 *        needs to be run once and never again, because it doesn't change.
 */
static void get_max_bl(void)
{
        char *path;
        FILE *max_bl_file;
        char buf[4];

        path = "/sys/devices/virtual/backlight/eeepc/max_brightness";
        max_bl_file = fopen(path, "r");
        if (!max_bl_file) {
                max_bl = NULL;
                return;
        }

        max_bl = fgets(buf, sizeof(buf), max_bl_file);
        fclose(max_bl_file);
        if (max_bl != NULL)
                chomp(max_bl);
}

