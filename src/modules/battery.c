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
#include "../lists.h"
#include "../mem.h"
#include "mod_utils.h"
#include "../util.h"

/* Module name */
char module_name[] = "battery";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function data structures and prototypes */
struct batt {
        char *num; /* should be "0", "1", etc. (as in BATT0, BATT1, etc.) */
        char *rem;  /* remaining charge in mAh */
        char *max;  /* maximum battery charge capacity in mAh */
};

char *get_battp(char *args);
char *get_battr(char *args);
char *get_battm(char *args);
int get_battb(char *args);

void batt_cron(void);
void clear_remaining_charge(struct batt *batt);

struct batt *prepare_batt(char *args);
struct batt *add_batt(char *args);
int find_batt(struct batt *cur, char *args);
char *get_remaining_charge(char *args);
char *get_max_charge(char *args);
void clear_batt(struct batt *cur);

/* Globals */
struct first_last *fl = NULL;

/* These run on module startup */
int module_init(void)
{
        fl = init_list();

        module_var_add(module_name, "battp", "get_battp", 30.0, VARIABLE_STR);
        module_var_add(module_name, "battr", "get_battr", 30.0, VARIABLE_STR);
        module_var_add(module_name, "battm", "get_battm", 30.0, VARIABLE_STR);

        module_var_add(module_name, "battb", "get_battb", 30.0, VARIABLE_BAR);

        module_var_cron_add(module_name,
                            "batt_cron",
                            "batt_cron",
                            30.0,
                            VARIABLE_CRON);
}

/* These run on module unload */
void module_destroy(void)
{
        del_list(&clear_batt, fl);
}

/** 
 * @brief Frees the recorded remaining charges for all batteries, forcing the
 *        next method(s) that call them to update their values until cron
 *        clears them again.
 */
void batt_cron(void)
{
        act_on_list(&clear_remaining_charge, fl);
}

/** 
 * @brief Callback for act_on_list() in batt_cron(). Frees and NULLs the
 *        remaining charge of the battery node passed to it.
 * 
 * @param batt Battery node to act upon.
 */
void clear_remaining_charge(struct batt *batt)
{
        if (batt->rem) {
                free(batt->rem);
                batt->rem = NULL;
        }
}

/** 
 * @brief Returns the remaining battery charge in percentage format.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battp 0} for BATT0)
 * 
 * @return Result as a string
 */
char *get_battp(char *args)
{
        struct batt *batt = prepare_batt(handle_args(args, "0"));
        if (batt == NULL)
                return m_strdup("n/a");

        int charge = (atof(batt->rem) / atof(batt->max)) * 100;

        char ret_battp[8];
        snprintf(ret_battp,
                 sizeof(ret_battp) - sizeof(char),
                 "%d",
                 charge);

        return m_strdup(ret_battp);
}

/** 
 * @brief Returns the remaining battery charge in raw mAh format.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battr 0} for BATT0)
 * 
 * @return Result as a string
 */
char *get_battr(char *args)
{
        struct batt *batt = prepare_batt(handle_args(args, "0"));
        if (batt == NULL)
                return m_strdup("n/a");

        return m_strdup(chomp(batt->rem));
}

/** 
 * @brief Returns the maximum battery charge in raw mAh format.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battm 0} for BATT0)
 * 
 * @return Result as a string
 */
char *get_battm(char *args)
{
        struct batt *batt = prepare_batt(handle_args(args, "0"));
        if (batt == NULL)
                return m_strdup("n/a");

        return m_strdup(chomp(batt->max));
}

/** 
 * @brief Returns the percentage of battery charge left as an int for use
 *        in drawing a bar.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battb 50 5 0} for
 *             a 50 pixel wide, 5 pixel high bar for BATT0)
 * 
 * @return Percentage of remaining battery charge as int
 */
int get_battb(char *args)
{
        struct batt *batt = prepare_batt(handle_args(args, "0"));
        if (batt == NULL)
                return 0;

        int charge = (atof(batt->rem) / atof(batt->max)) * 100;

        return charge;
}

/** 
 * @brief Fetches a node for the requested battery.
 * 
 * @param batt_num Battery number to prepare.
 * 
 * @return NULL if a battery's remaining or maximum charge could not be
 *         determined. Otherwise, a pointer to the battery is returned.
 */
struct batt *prepare_batt(char *batt_num)
{
        struct batt *batt = get_node(&find_batt, &add_batt, batt_num, fl);

        if (batt->rem == NULL)
                batt->rem = get_remaining_charge(batt->num);
        if ((batt->rem == NULL) || (batt->max == NULL))
                return NULL;

        return batt;
}

/** 
 * @brief Adds a new battery node to the list.
 * 
 * @param batt_num Battery number to add. 
 * 
 * @return Pointer to the new node.
 */
struct batt *add_batt(char *batt_num)
{
        struct batt *new = malloc(sizeof(struct batt));
        new->num = NULL;
        new->rem = NULL;
        new->max = NULL;

        new->num = d_strcpy(batt_num);

        /* this never needs to be updated */
        new->max = get_max_charge(new->num);

        return add_node(new, fl);
}

/** 
 * @brief Callback for get_node() in lists.h
 * 
 * @param batt Battery node whose name we match against
 * @param match The name of the node we're looking for
 * 
 * @return 1 if match succeeds, 0 if fails
 */
int find_batt(struct batt *batt, char *match)
{
        if (!strcmp(batt->num, match))
                return 1;
        else
                return 0;
}

/** 
 * @brief Retrieves the remaining charge for a battery from /sys
 * 
 * @param args Battery name (number) whose charge we want.
 * 
 * @return Remaining mAh charge in string format.
 */
char *get_remaining_charge(char *args)
{
        char path[64];
        snprintf(path,
                 sizeof(path) - sizeof(char),
                 "/sys/class/power_supply/BAT%s/charge_now",
                 args);

        FILE *rem_charge = fopen(path, "r");
        if (rem_charge == NULL)
                return NULL;

        char rem[16];
        if (fgets(rem, 16, rem_charge) == NULL) {
                fclose(rem_charge);
                return NULL;
        }

        fclose(rem_charge);

        return d_strcpy(rem);
}

/** 
 * @brief Retrieves the maximum charge for a battery from /sys.
 *        This is only invoked once per battery in add_batt()
 *        because it never changes, obviously.
 * 
 * @param args Battery name (number) whose charge we want.
 * 
 * @return Maximum mAh charge in string format.
 */
char *get_max_charge(char *args)
{
        char path[64];
        snprintf(path,
                 sizeof(path) - sizeof(char),
                 "/sys/class/power_supply/BAT%s/charge_full",
                 args);

        FILE *max_charge;
        max_charge = fopen(path, "r");
        if (max_charge == NULL)
                return NULL;

        char max[16];
        if (fgets(max, 16, max_charge) == NULL) {
                fclose(max_charge);
                return NULL;
        }

        fclose(max_charge);

        return d_strcpy(max);
}

/** 
 * @brief Frees the contents of a battery node.
 * 
 * @param batt Battery node to clear.
 */
void clear_batt(struct batt *batt)
{
        freeif(batt->num);
        freeif(batt->rem);
        freeif(batt->max);
}

