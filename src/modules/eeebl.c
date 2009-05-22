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

#include "../c99.h"
#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "eeebl";

/* My function prototypes */
char *get_eeeblper(char *args);
char *get_eeeblcur(char *args);
char *get_eeeblmax(char *args);
unsigned int get_eeeblbar(char *args);

void get_cur_bl(void);
void get_max_bl(void);

/* Globals */
char *cur_bl = NULL;
char *max_bl = NULL;

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "eeeblper", "get_eeeblper", 5.0, VARIABLE_STR);
        module_var_add(mod, "eeeblcur", "get_eeeblcur", 5.0, VARIABLE_STR);
        module_var_add(mod, "eeeblmax", "get_eeeblmax", 5.0, VARIABLE_STR);
        module_var_add(mod, "eeeblbar", "get_eeeblbar", 5.0, VARIABLE_BAR);
        module_var_add(mod, "eeebl_cron", "eeebl_cron", 5.0, VARIABLE_CRON);
        get_max_bl();
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
        freenull(cur_bl);
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
unsigned int get_eeeblbar(char *args)
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

