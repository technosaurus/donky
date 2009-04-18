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
#include <getopt.h>
#include <signal.h>
#include <xcb/xcb.h>

#include "../config.h"
#include "config.h"
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
void initialize_stuff(void);
void clean_up_everything(void);
int main(int argc, char **argv);
void sigterm_handler(int signum);
void sighup_handler(int signum);
void sigint_handler(int signum);

/* Globals. */
int donky_reload = 0;
int donky_exit = 0;

struct x_connection *xc;
struct window_settings *ws;

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
                { "version", no_argument,       0, 'v' },
                { "help",    no_argument,       0, 'h' },
                { "config",  required_argument, 0, 'c' },
                { "debug",   no_argument,       0, 'd' },
                { 0, 0, 0, 0 }
        };

        int option_index = 0;
        int c = 0;

        while (1) {
                c = getopt_long(argc, argv, "vhc:d",
                                long_options, &option_index);

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
void initialize_stuff(void)
{
        while (1) {
                xc = init_x_connection();
                printf("made X connection\n");

                parse_cfg();
                printf("parsed config\n");

                parse_text();
                printf("parsed [text]\n");

                module_load_all();
                printf("loaded modules\n");

                ws = draw_window(xc);
                printf("drew window\n");

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
void clean_up_everything(void)
{
        extern struct first_last *cfg_fl;
        extern struct first_last *ts_fl;

        del_list(cfg_fl, &clear_cfg);
        //clear_text();
        del_list(ts_fl, &clear_text);
        clear_module();
}

/**
 * @brief Handles SIGTERM signal.
 */
void sigterm_handler(int signum)
{
        donky_exit = 1;
}

/**
 * @brief Handles SIGHUP signal.
 */
void sighup_handler(int signum)
{
        donky_reload = 1;
}

/**
 * @brief Handles SIGINT signal.
 */
void sigint_handler(int signum)
{
        donky_exit = 1;
}
