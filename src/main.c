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

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "../config.h"
#include "c99.h"
#include "config.h"
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
        while (1) {
                donky_greet();

                printf("Parsing .donkyrc...\n");
                parse_cfg();

                printf("Loading modules...\n");
                module_load_all();

                printf("Starting the donky loop (TM)... >_<\n");
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
        extern struct list *cfg_ls;

        printf("Clearing config list...");
        del_list(cfg_ls, &clear_cfg);
        printf(" done.\n");

        printf("Clearing modules...");
        clear_module();
        printf(" done.\n");

        printf("Clearing request list...");
        request_list_clear();
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
        const char *greetings[] = {
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
        const char *farewells[] = {
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
static void print_random_message(const char **messages)
{
        int i;
        int j;

        for (i = 0; messages[i] != NULL; i++) { }

        if (i) {
                j = random_range(0, (i - 1));
                printf(messages[j]);
        }
}

