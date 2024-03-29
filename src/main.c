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

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "cfg.h"
#include "daemon.h"
#include "main.h"
#include "module.h"
#include "request.h"
#include "util.h"

#define HELP \
        "donky usage:\n" \
        "  -v, --version        Show donky version\n" \
        "  -h, --help           Show this information\n" \
        "  -c, --config=FILE    Use alternate configuration file\n" \
        "  -d, --debug          Show debugging messages\n"

/* Function prototypes. */
int main(int argc, char **argv);
static void initialize_stuff(void);
static void clean_up_everything(void);
static void sigterm_handler(int signum);
static void sighup_handler(int signum);
static void sigint_handler(int signum);
static void donky_greet(void);
static void donky_farewell(void);
static void print_random_message(const char **messages);

/* Globals. */
int donky_reload;
int donky_exit;

/**
 * @brief Program entry point.
 *
 * @param argc Argument count
 * @param argv Array of arguments
 * 
 * @return Exit status
 */
int main(int argc, char **argv)
{
        static struct option long_options[] = {
                { "version", no_argument,       NULL, 'v' },
                { "help",    no_argument,       NULL, 'h' },
                { "config",  required_argument, NULL, 'c' },
                { "debug",   no_argument,       NULL, 'd' },
                { NULL,      0,                 NULL,  0  }
        };

        int option_index = 0;
        int c = 0;

        donky_reload = 0;
        donky_exit = 0;

        while (1) {
                c = getopt_long(argc,
                                argv,
                                "vhc:d",
                                long_options,
                                &option_index);

                if (c == -1)
                        break;

                switch (c) {
                case 'v':
                        printf("donky %s\n", VERSION);
                        exit(EXIT_SUCCESS);
                case 'h':
                        printf(HELP);
                        exit(EXIT_SUCCESS);
                case 'c':
                        printf("Using alternate config file: %s\n", optarg);
                        break;
                default:
                        printf("\n" HELP);
                        exit(EXIT_FAILURE);
                }
        }

        /* Set signal handlers. */
        signal(SIGTERM, sigterm_handler);
        signal(SIGHUP, sighup_handler);
        signal(SIGINT, sigint_handler);
        signal(SIGPIPE, SIG_IGN);

        /* Initialize, then start donky. */
        initialize_stuff();

        return 0;
}

/**
 * @brief Initialize everything donky needs to next begin drawing.
 */
static void initialize_stuff(void)
{
        while (1) {
                donky_greet();

                DEBUGF(("Parsing .donkyrc...\n"));
                parse_cfg();

                DEBUGF(("Loading modules...\n"));
                module_load_all();

                DEBUGF(("Starting the donky loop (TM)... >_<\n"));
                donky_loop();

                clean_up_everything();

                /* Exit flag! */
                if (donky_exit)
                        break;

                /* We just reloaded, negate the flag. */
                if (donky_reload)
                        donky_reload = 0;
        }
}

/**
 * @brief Run all cleanup methods for (each) source file.
 */
static void clean_up_everything(void)
{
        DEBUGF(("Clearing config list..."));
        clear_cfg();
        DEBUGF((" done.\n"));

        DEBUGF(("Clearing modules..."));
        clear_module();
        DEBUGF((" done.\n"));

        DEBUGF(("Clearing request list..."));
        request_list_clear();
        DEBUGF((" done.\n"));
}

/**
 * @brief Handles SIGTERM signal.
 */
static void sigterm_handler(int signum)
{
        donky_farewell();
        donky_exit = 1;
}

/**
 * @brief Handles SIGHUP signal.
 */
static void sighup_handler(int signum)
{
        donky_farewell();
        donky_reload = 1;
}

/**
 * @brief Handles SIGINT signal.
 */
static void sigint_handler(int signum)
{
        donky_farewell();
        donky_exit = 1;
}

/** 
 * @brief Prints a random donky greeting!!!!!! Wowwee
 */
static void donky_greet(void)
{
        const char *greetings[] = {
                "Welcome to donky! Have a mint.",
                "SUP! donky in the h00se.",
                "Who dare awakes donky? Mothereff.",
                "donky. It's like honky, minus racial connotations.",
                "My name is donky, and I am your g0d.",
                "donky hygiene tip: shower before I segfault.",
                "donky is starting... beginning... going... doing shit...",
                NULL /* always last */
        };

        print_random_message(greetings);
}

/** 
 * @brief donky is so nice, it says bye!
 */
static void donky_farewell(void)
{
        const char *farewells[] = {
                "donky is peacin' the eff out.",
                "I'm OUT!",
                "donky is out like California lights.",
                "Okay... donky knows when it's not wanted... (//_.)",
                "donky sez, \"PTFO!\"",
                "KBYE. >:O",
                NULL /* always last */
        };

        print_random_message(farewells);
}

/** 
 * @brief Prints a random string from a string pointer array.
 * 
 * @param messages An array of char pointers with the last pointer = NULL
 *                 (so we can count the number of elements!!!!1)
 */
static void print_random_message(const char **messages)
{
        int i, j;

        /* count the number of strings in the array */
        for (i = 0; messages[i] != NULL; i++);

        /* --i since arrays obviously start at 0, not 1 */
        if (--i >= 0) {
                j = random_range(0, i);
                printf("%s\n", messages[j]);
        }
}

