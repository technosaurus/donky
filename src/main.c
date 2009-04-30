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

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>

#include "../config.h"
#include "config.h"
#include "main.h"
#include "module.h"
#include "render.h"
#include "text.h"
#include "util.h"
#include "x11.h"

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
static void print_random_message(char **messages);

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
                { NULL,         0,              NULL,  0  }
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
                case 'd':
                        printf("Debug mode enabled.");
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

        /* Initialize, then start donky. */
        initialize_stuff();

        exit(EXIT_SUCCESS);
}

/**
 * @brief Initialize everything donky needs to next begin drawing.
 */
static void initialize_stuff(void)
{
        struct x_connection *xc;
        struct window_settings *ws;

        while (1) {
                donky_greet();

                printf("Making X connection...\n");
                xc = init_x_connection();

                printf("Parsing .donkyrc...\n");
                parse_cfg();

                printf("Parsing [text]...\n");
                parse_text();

                printf("Loading modules...\n");
                module_load_all();

                printf("Drawing donky's window...\n");
                ws = draw_window(xc);

                printf("Starting the donky loop (TM)... >_<\n");
                donky_loop(xc, ws);

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
        extern struct list *cfg_ls;
        extern struct list *ts_ls;

        printf("Clearing config list...");
        del_list(cfg_ls, &clear_cfg);
        printf(" done.\n");

        printf("Clearing text_section list...");
        del_list(ts_ls, &clear_text);
        printf(" done.\n");

        printf("Clearing modules...");
        clear_module();
        printf(" done.\n");
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
        char *greetings[] = {
                "Welcome to donky! Have a mint.\n",
                "SUP! donky in the h00se.\n",
                "Who dare awakes donky? Mothereff.\n",
                "donky. It's like honky, minus racial connotations.\n",
                "My name is donky, and I am your g0d.\n",
                "donky hygiene tip: shower before I segfault.\n",
                "donky is starting... beginning... going... doing shit...\n",
                NULL /* always last */
        };

        print_random_message(greetings);
}

/** 
 * @brief donky is so nice, it says bye!
 */
static void donky_farewell(void)
{
        char *farewells[] = {
                "donky is peacin' the eff out.\n",
                "I'm OUT!\n",
                "donky is out like California lights.\n",
                "Okay... donky knows when it's not wanted... (//_.)\n",
                "donky sez, \"PTFO!\"\n",
                "KBYE. >:O\n",
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
static void print_random_message(char **messages)
{
        int i;
        int j;

        for (i = 0; messages[i] != NULL; i++) { }

        if (i) {
                j = random_range(0, (i - 1));
                printf(messages[j]);
        }
}

