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
#include <string.h>

#include "../lists.h"
#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "battery";

/* My function data structures and prototypes */
struct batt {
        char *number;           /* battery number ("0" for BATT0) */

        char *remaining; /* remaining charge in mAh */
        char *maximum;   /* maximum charge capacity in mAh */
};

static void clear_remaining(struct batt *batt);

static struct batt *prepare_batt(char *args);
static int find_batt(struct batt *cur, char *args);
static struct batt *add_batt(char *args);
static char *get_remaining_charge(char *args);
static char *get_maximum_charge(char *args);

static void clear_batt(void *cur);

/* Globals */
struct list *batt_ls = NULL;

/* These run on module startup */
void module_init(const struct module *mod)
{
        module_var_add(mod, "battper", "get_battper", 30.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "battrem", "get_battrem", 30.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "battmax", "get_battmax", 30.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "battbar", "get_battbar", 30.0, VARIABLE_BAR | ARGSTR);
        module_var_add(mod, "batt_cron", "batt_cron", 30.0, VARIABLE_CRON);
}

/* These run on module unload */
void module_destroy(void)
{
        del_list(batt_ls, &clear_batt);
}

/** 
 * @brief Frees the recorded remaining charges for all batteries, forcing the
 *        next method(s) that call them to update their values until cron
 *        clears them again.
 */
void batt_cron(void)
{
        if (batt_ls == NULL)
                batt_ls = init_list();

        act_on_list(batt_ls, &clear_remaining);
}

/** 
 * @brief Callback for act_on_list() in batt_cron(). Frees and NULLs the
 *        remaining charge of the battery node passed to it.
 * 
 * @param batt Battery node to act upon.
 */
static void clear_remaining(struct batt *batt)
{
        freenull(batt->remaining);
}

/** 
 * @brief Returns the remaining battery charge in percentage format.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battper 0} for BATT0)
 * 
 * @return Result as a string
 */
char *get_battper(char *args)
{
        struct batt *batt;
        int percentage;
        char battper[4];

        batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) ||
            (batt->remaining == NULL) || (batt->maximum == NULL))
                return "n/a";

        percentage = (atof(batt->remaining) / atof(batt->maximum)) * 100;
        snprintf(battper, sizeof(battper), "%d", percentage);

        return m_strdup(battper);
}

/** 
 * @brief Returns the remaining battery charge in raw mAh format.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battrem 0} for BATT0)
 * 
 * @return Result as a string
 */
char *get_battrem(char *args)
{
        struct batt *batt;
        
        batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) || (batt->remaining == NULL))
                return "n/a";

        return batt->remaining;
}

/** 
 * @brief Returns the maximum battery charge in raw mAh format.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battmax 0} for BATT0)
 * 
 * @return Result as a string
 */
char *get_battmax(char *args)
{
        struct batt *batt;
        
        batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) || (batt->maximum == NULL))
                return "n/a";

        return batt->maximum;
}

/** 
 * @brief Returns the percentage of battery charge left as an int for use
 *        in drawing a bar.
 * 
 * @param args Module arguments from .donkyrc (e.g. ${battbar 50 5 0} for
 *             a 50 pixel wide, 5 pixel high bar for BATT0)
 * 
 * @return Percentage of remaining battery charge as int
 */
unsigned int get_battbar(char *args)
{
        struct batt *batt;
        int percentage;

        batt = prepare_batt((args) ? args : "0");
        if ((batt == NULL) ||
            (batt->remaining == NULL) || (batt->maximum == NULL))
                return 0;

        percentage = (atof(batt->remaining) / atof(batt->maximum)) * 100;

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
static struct batt *prepare_batt(char *batt_number)
{
        struct batt *batt;

        batt = get_node(batt_ls, &find_batt, batt_number, &add_batt);
        if (batt->remaining == NULL)
                batt->remaining = get_remaining_charge(batt->number);

        return batt;
}

/** 
 * @brief Adds a new battery node to the list.
 * 
 * @param batt_num Battery number to add. 
 * 
 * @return Pointer to the new node.
 */
static struct batt *add_batt(char *batt_number)
{
        struct batt *new;

        new = malloc(sizeof(struct batt));

        new->number = dstrdup(batt_number);
        new->remaining = NULL;
        new->maximum = get_maximum_charge(new->number);

        return add_node(batt_ls, new);
}

/** 
 * @brief Callback for get_node() in lists.h
 * 
 * @param batt Battery node whose name we match against
 * @param match The name of the node we're looking for
 * 
 * @return 1 if match succeeds, 0 if fails
 */
static int find_batt(struct batt *cur, char *match)
{
        if (!strcmp(cur->number, match))
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
static char *get_remaining_charge(char *args)
{
        char path[DMAXPATHLEN];
        int read;
        FILE *fptr;
        char remaining[16];
        char *fgets_check;

        read = snprintf(path, sizeof(path), 
                        "/sys/class/power_supply/BAT%s/charge_now", args);

        if (read >= DMAXPATHLEN)
                printf("WARNING: [battery:get_remaining_charge] "
                       "snprintf truncation! read = %d, DMAXPATHLEN = %d!\n",
                       read, DMAXPATHLEN);

        fptr = fopen(path, "r");
        if (fptr == NULL)
                return NULL;

        fgets_check = fgets(remaining, sizeof(remaining), fptr);
        fclose(fptr);
        if (fgets_check == NULL)
                return NULL;

        chomp(remaining);

        return dstrdup(remaining);
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
static char *get_maximum_charge(char *args)
{
        char path[DMAXPATHLEN];
        int read;
        FILE *fptr;
        char maximum[16];
        char *fgets_check;

        read = snprintf(path, sizeof(path), 
                        "/sys/class/power_supply/BAT%s/charge_full", args);

        if (read >= DMAXPATHLEN)
                printf("WARNING: [battery:get_remaining_charge] "
                       "snprintf truncation! read = %d, DMAXPATHLEN = %d!\n",
                       read, DMAXPATHLEN);

        fptr = fopen(path, "r");
        if (fptr == NULL)
                return NULL;

        fgets_check = fgets(maximum, sizeof(maximum), fptr);
        fclose(fptr);
        if (fgets_check == NULL)
                return NULL;

        chomp(maximum);

        return dstrdup(maximum);
}

/** 
 * @brief Frees the contents of a battery node.
 * 
 * @param batt Battery node to clear.
 */
static void clear_batt(void *cur)
{
        struct batt *batt = cur;

        free(batt->number);
        free(batt->remaining);
        free(batt->maximum);
}

