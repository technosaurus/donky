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
#include <string.h>

#include "../mem.h"
#include "../module.h"
#include "../util.h"

/* Module name */
char module_name[] = "battery";

/* Data structures and prototypes */
struct batt {
        long number;            /* battery number. BAT0 is 0 */
        char *file_remaining;   /* file name of "remaining charge" file */
        char *remaining;        /* remaining charge in mAh */
        char *file_maximum;     /* file name of "maximum charge" file */
        char *maximum;          /* maximum charge capacity in mAh */

        struct batt *next;
};

struct batt_ls {
        struct batt *first;
        struct batt *last;
};

static void init_batt_list(void);
void batt_cron(void);
static struct batt *prepare_batt(const char *args);
static struct batt *add_batt(const char *batt_number, long batt_number_int);
static char *get_charge(const char *path);

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

        for (cur = batt_ls->first; cur != NULL; cur = next) {
                next = cur->next;
                free(cur->file_remaining);
                free(cur->remaining);
                free(cur->file_maximum);
                free(cur->maximum);
                free(cur);
        }

        freenull(batt_ls);
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

        for (cur = batt_ls->first; cur != NULL; cur = cur->next)
                freenull(cur->remaining);
}

/**
 * @brief Calculate and return the remaining charge of a battery in
 *        percentage format as a string.
 */
char *get_battper(char *args)
{
        struct batt *batt;
        unsigned int percentage;
        char battper[4];

        batt = prepare_batt(args);
        if ((batt == NULL) ||
            (batt->remaining == NULL) || (batt->maximum == NULL))
                return "n/a";

        percentage = (atof(batt->remaining) / atof(batt->maximum)) * 100;
        snprintf(battper, sizeof(battper), "%u", percentage);

        return m_strdup(battper);
}

/**
 * @brief Returns the remaining charge of a battery in raw mAh as a string.
 */
char *get_battrem(char *args)
{
        struct batt *batt;
        
        batt = prepare_batt(args);
        if ((batt == NULL) || (batt->remaining == NULL))
                return "n/a";

        return batt->remaining;
}

/**
 * @brief Returns the maximum charge of a battery in raw mAh as a string.
 */
char *get_battmax(char *args)
{
        struct batt *batt;

        batt = prepare_batt(args);
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

        batt = prepare_batt(args);
        if ((batt == NULL) ||
            (batt->remaining == NULL) || (batt->maximum == NULL))
                return 0;

        percentage = (atof(batt->remaining) / atof(batt->maximum)) * 100;

        return percentage;
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
 * @brief Looks for a battery in the list and updates its remaining charge if
 *        needed. If the battery isn't in the list, it is added.
 */
static struct batt *prepare_batt(const char *args)
{
        extern struct batt_ls *batt_ls;
        struct batt *cur;
        const char *batt_number;
        long batt_number_int;

        batt_number = (args !=  NULL) ? args : "0";
        batt_number_int = strtol(batt_number, NULL, 10);

        for (cur = batt_ls->first; cur != NULL; cur = cur->next)
                if (cur->number == batt_number_int)
                        break;

        if (cur == NULL)
                cur = add_batt(batt_number, batt_number_int);
        if (cur->remaining == NULL)
                cur->remaining = get_charge(cur->file_remaining);

        return cur;
}

/**
 * @brief Adds a new battery to the list.
 */
#define BAT_PATH "/sys/class/power_supply/BAT"
#define REM_FILE "/charge_now"
#define MAX_FILE "/charge_full"
static struct batt *add_batt(const char *batt_number, long batt_number_int)
{
        extern struct batt_ls *batt_ls;
        struct batt *new;

        new = malloc(sizeof(struct batt));
        new->number = batt_number_int;
        new->next = NULL;

        /* build paths of /sys battery charge file */
        asprintf(&new->file_remaining, BAT_PATH "%s" REM_FILE, batt_number);
        asprintf(&new->file_maximum, BAT_PATH "%s" MAX_FILE, batt_number);

        new->remaining = NULL;  /* to be filled by prepare_batt() */
        new->maximum = get_charge(new->file_maximum);

        if (batt_ls->last != NULL) {
                batt_ls->last->next = new;
                batt_ls->last = new;
        } else {
                batt_ls->first = new;
                batt_ls->last = new;
        }

        return batt_ls->last;   /* return the new node */
}

/**
 * @brief Retrieves the charge for a battery from a /sys file
 */
static char *get_charge(const char *path)
{
        FILE *fptr;
        char remaining[16];
        char *fgets_check;

        fptr = fopen(path, "r");
        if (fptr == NULL)
                return NULL;

        fgets_check = fgets(remaining, sizeof(remaining), fptr);
        fclose(fptr);
        if (fgets_check == NULL)
                return NULL;

        chomp(remaining);

        return strdup(remaining);
}

