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
#include "../util.h"

/* Module name */
char module_name[] = "battery";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function data structures and prototypes */
struct batt {
        char *number;           /* battery number ("0" for BATT0) */

        char *remaining_charge; /* remaining charge in mAh */
        char *maximum_charge;   /* maximum charge capacity in mAh */
};

char *get_battp(char *args);
char *get_battr(char *args);
char *get_battm(char *args);
int get_battb(char *args);

void batt_cron(void);
void clear_remaining_charge(struct batt *batt);

struct batt *prepare_batt(char *args);
int find_batt(struct batt *cur, char *args);
struct batt *add_batt(char *args);
char *get_remaining_charge(char *args);
char *get_maximum_charge(char *args);

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
        del_list(fl, &clear_batt);
}

/** 
 * @brief Frees the recorded remaining charges for all batteries, forcing the
 *        next method(s) that call them to update their values until cron
 *        clears them again.
 */
void batt_cron(void)
{
        act_on_list(fl, &clear_remaining_charge);
}

/** 
 * @brief Callback for act_on_list() in batt_cron(). Frees and NULLs the
 *        remaining charge of the battery node passed to it.
 * 
 * @param batt Battery node to act upon.
 */
void clear_remaining_charge(struct batt *batt)
{
        if (batt->remaining_charge) {
                free(batt->remaining_charge);
                batt->remaining_charge = NULL;
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
        struct batt *batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) ||
            (batt->remaining_charge == NULL) || (batt->maximum_charge == NULL))
                return m_strdup("n/a");

        double rem = atof(batt->remaining_charge);
        double max = atof(batt->maximum_charge);
        int percentage = (rem / max) * 100;

        char *battp = NULL;
        if (asprintf(&battp, "%d", percentage) == -1)
                return m_strdup("n/a");

        return m_freelater(battp);
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
        struct batt *batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) || (batt->remaining_charge == NULL))
                return m_strdup("n/a");

        return m_strdup(chomp(batt->remaining_charge));
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
        struct batt *batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) || (batt->maximum_charge == NULL))
                return m_strdup("n/a");

        return m_strdup(chomp(batt->maximum_charge));
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
        struct batt *batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) ||
            (batt->remaining_charge == NULL) || (batt->maximum_charge == NULL))
                return 0;

        double rem = atof(batt->remaining_charge);
        double max = atof(batt->maximum_charge);
        int percentage = (rem / max) * 100;

        return percentage;
}

/** 
 * @brief Fetches a node for the requested battery.
 * 
 * @param batt_num Battery number to prepare.
 * 
 * @return NULL if a battery's remaining or maximum charge could not be
 *         determined. Otherwise, a pointer to the battery is returned.
 */
struct batt *prepare_batt(char *batt_number)
{
        struct batt *batt = get_node(fl, &find_batt, batt_number, &add_batt);
        if (batt->remaining_charge == NULL)
                batt->remaining_charge = get_remaining_charge(batt->number);

        return batt;
}

/** 
 * @brief Adds a new battery node to the list.
 * 
 * @param batt_num Battery number to add. 
 * 
 * @return Pointer to the new node.
 */
struct batt *add_batt(char *batt_number)
{
        struct batt *new = malloc(sizeof(struct batt));
        new->number = NULL;
        new->remaining_charge = NULL;
        new->maximum_charge = NULL;

        new->number = d_strcpy(batt_number);
        new->maximum_charge = get_maximum_charge(new->number);

        return add_node(fl, new);
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
        if (!strcmp(batt->number, match))
                return 1;

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
        char *path = NULL;
        if (asprintf(&path,
                     "/sys/class/power_supply/BAT%s/charge_now",
                     args) == -1)
                return NULL;

        FILE *fptr = fopen(path, "r");
        free(path);
        if (fptr == NULL)
                return NULL;

        char *remaining_charge = NULL;
        int len = 0; /* forces a malloc */
        int read = getline(&remaining_charge, &len, fptr);
        fclose(fptr);
        if (read == -1)
                return NULL;

        return remaining_charge;
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
char *get_maximum_charge(char *args)
{
        char *path = NULL;
        if (asprintf(&path,
                     "/sys/class/power_supply/BAT%s/charge_full",
                     args) == -1)
                return NULL;

        FILE *fptr = fopen(path, "r");
        free(path);
        if (fptr == NULL)
                return NULL;

        char *maximum_charge = NULL;
        int len = 0; /* forces a malloc */
        int read = getline(&maximum_charge, &len, fptr);
        fclose(fptr);
        if (read == -1)
                return NULL;

        return maximum_charge;
}

/** 
 * @brief Frees the contents of a battery node.
 * 
 * @param batt Battery node to clear.
 */
void clear_batt(struct batt *batt)
{
        freeif(batt->number);
        freeif(batt->remaining_charge);
        freeif(batt->maximum_charge);
}

