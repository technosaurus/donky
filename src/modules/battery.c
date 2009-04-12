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
#include "../util.h"

/* Module name */
char module_name[] = "battery";

/* Required function prototypes. */
int module_init(void);
void module_destroy(void);

/* My function data structures and prototypes */
struct batt {
        char *name; /* should be "0", "1", etc. (as in BATT0, BATT1, etc.) */
        char *rem;  /* remaining charge in mAh */
        char *max;  /* maximum battery charge capacity in mAh */

        struct batt *next;
};

char *get_battp(char *args);
char *get_battr(char *args);
char *get_battm(char *args);
int get_battb(char *args);

void batt_cron(void);

struct batt *prepare_batt(char *args);
struct batt *add_batt(char *args);
struct batt *get_batt(char *args);
char *get_remaining_charge(char *args);
char *get_max_charge(char *args);
void clear_batt(void);

/* Globals */
struct batt *first_batt = NULL;
struct batt *last_batt = NULL;

/* These run on module startup */
int module_init(void)
{
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
        clear_batt();
}

/** 
 * @brief Frees the recorded remaining charges for all batteries, forcing the
 *        next method(s) that call them to update their values until cron
 *        clears them again.
 */
void batt_cron(void)
{
        struct batt *cur = first_batt;
        struct batt *next;

        while (cur) {
                next = cur->next;

                freeif(cur->rem);
                cur->rem = NULL;

                cur = next;
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
        struct batt *batt = prepare_batt(args);
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
        struct batt *batt = prepare_batt(args);
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
        struct batt *batt = prepare_batt(args);
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
        struct batt *batt = prepare_batt(args);
        if (batt == NULL)
                return 0;

        int charge = (atof(batt->rem) / atof(batt->max)) * 100;

        return charge;
}

/** 
 * @brief Fetches a node for the requested battery (defaults to BATT0 if
 *        no battery specified).
 * 
 * @param args Module args.
 * 
 * @return NULL if a battery's remaining or maximum charge could not be
 *         determined. Otherwise, a pointer to the battery is returned.
 */
struct batt *prepare_batt(char *args)
{
        char *batt_num = NULL;
        if (args)
                batt_num = args;
        else
                batt_num = "0";

        struct batt *batt = get_batt(batt_num);
        if (batt->rem == NULL)
                batt->rem = get_remaining_charge(batt_num);
        if (batt->rem == NULL || (batt->max == NULL))
                return NULL;

        return batt;
}

/** 
 * @brief Adds a new node to the battery linked list.
 * 
 * @param args Battery name (number) to add.
 * 
 * @return Pointer to the new battery node.
 */
struct batt *add_batt(char *args)
{
        struct batt *new_batt = malloc(sizeof(struct batt));
        new_batt->name = NULL;
        new_batt->rem = NULL;
        new_batt->max = NULL;
        new_batt->next = NULL;

        new_batt->name = d_strcpy(args);

        /* this never needs to be updated */
        new_batt->max = get_max_charge(new_batt->name);

        if (last_batt) {
                last_batt->next = new_batt;
                last_batt = new_batt;
        } else {
                first_batt = last_batt = new_batt;
        }

        return new_batt;
}

/** 
 * @brief Fetches a pointer to the requested battery node. If the node
 *        doesn't exist, it invokes add_batt() and returns the new node.
 * 
 * @param args Battery name (number) to look up.
 * 
 * @return Pointer to the requested node.
 */
struct batt *get_batt(char *args)
{
        struct batt *cur = first_batt;

        while (cur) {
                if (!strcmp(cur->name, args))
                        return cur;

                cur = cur->next;
        }

        return add_batt(args);
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
 * @brief Frees the battery linked list and its contents from memory.
 */
void clear_batt(void)
{
        struct batt *cur = first_batt;
        struct batt *next;

        while (cur) {
                next = cur->next;

                freeif(cur->name);
                freeif(cur->rem);
                freeif(cur->max);
                freeif(cur);

                cur = next;
        }

        first_batt = NULL;
        last_batt = NULL;
}

