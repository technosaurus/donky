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

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "battery";

/* My function data structures and prototypes */
struct batt {
        char *number;           /* battery number ("0" for BATT0) */
        char *remaining;        /* remaining charge in mAh */
        char *maximum;          /* maximum charge capacity in mAh */

        struct batt *next;
};

struct batt_ls {
        struct batt *first;
        struct batt *last;
};

static void init_batt_list(void);
void batt_cron(void);
static struct batt *prepare_batt(char *args);
static struct batt *get_batt(char *batt_number);
static struct batt *add_batt(char *batt_number);
static char *get_remaining_charge(char *args);
static char *get_maximum_charge(char *args);

/* Globals */
struct batt_ls *batt_ls = NULL;

/* This runs on module startup */
void module_init(const struct module *mod)
{
        init_batt_list();
        module_var_add(mod, "battper", "get_battper", 30.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "battrem", "get_battrem", 30.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "battmax", "get_battmax", 30.0, VARIABLE_STR | ARGSTR);
        module_var_add(mod, "battbar", "get_battbar", 30.0, VARIABLE_BAR | ARGSTR);
        module_var_add(mod, "batt_cron", "batt_cron", 30.0, VARIABLE_CRON);
}

/* This runs on module unload */
void module_destroy(void)
{
        extern struct batt_ls *batt_ls;
        struct batt *cur;
        struct batt *next;

        cur = batt_ls->first;
        while (cur != NULL) {
                next = cur->next;
                free(cur->number);
                free(cur->remaining);
                free(cur->maximum);
                cur = next;
        }

        freenull(batt_ls);
}

/**
 * @brief Initializes the module's global battery list
 */
static void init_batt_list(void)
{
        extern struct batt_ls *batt_ls;

        batt_ls = malloc(sizeof(struct batt_ls));
        batt_ls->first = NULL;
        batt_ls->last = NULL;
}

/**
 * @brief This clears the "remaining charge" of every battery in the
 *        list, forcing the next function that wants the value to
 *        update it first.
 */
void batt_cron(void)
{
        extern struct batt_ls *batt_ls;
        struct batt *cur;

        cur = batt_ls->first;
        while (cur != NULL) {
                freenull(cur->remaining);
                cur = cur->next;
        }
}

/**
 * @brief Calculate and return the remaining charge of a battery in
 *        percentage format.
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
 * @brief Returns the remaining charge of a battery in raw mAh.
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
 * @brief Returns the maximum charge of a battery in raw mAh.
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
 * @brief Returns the percentage of the remaining charge of a battery
 *        as in int for use in drawing a bar.
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
 * @brief Finds a battery in the list and updates its remaining charge
 *        if need be.
 */
static struct batt *prepare_batt(char *batt_number)
{
        struct batt *batt;

        batt = get_batt(batt_number);
        if (batt->remaining == NULL)
                batt->remaining = get_remaining_charge(batt->number);

        return batt;
}

/**
 * @brief Used by prepare_batt() to find the requested battery. If it
 *        cannot be found, add_batt() is called to add the battery to
 *        the list and then return the newly created battery node.
 */
static struct batt *get_batt(char *batt_number)
{
        extern struct batt_ls *batt_ls;
        struct batt *cur;

        cur = batt_ls->first;
        while (cur != NULL) {
                if (!strcmp(cur->number, batt_number))
                        return cur;
                cur = cur->next;
        }

        return add_batt(batt_number);
}

/**
 * @brief Adds a new battery to the list.
 */
static struct batt *add_batt(char *batt_number)
{
        extern struct batt_ls *batt_ls;
        struct batt *new;

        new = malloc(sizeof(struct batt));
        new->number = dstrdup(batt_number);
        new->remaining = NULL;
        new->maximum = get_maximum_charge(new->number);

        if (batt_ls->last != NULL) {
                batt_ls->last->next = new;
                batt_ls->last = new;
        } else {
                batt_ls->first = new;
                batt_ls->last = new;
        }

        return batt_ls->last;
}

/**
 * @brief Retrieves the remaining charge for a battery from /sys
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

