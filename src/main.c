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
#include <xcb/xcb.h>

#include "../config.h"
#include "config.h"
#include "text.h"
#include "module.h"
#include "util.h"
#include "x11.h"
#include "render.h"

#define HELP \
        "donky usage:\n" \
        "  -v, --version        Show donky version\n" \
        "  -h, --help           Show this information\n" \
        "  -c, --config=FILE    Use alternate configuration file\n" \
        "  -d, --debug          Show debugging messages\n"

void initialize_stuff(void);
void clean_up_everything(void);
int main(int argc, char **argv);

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

        /* Initialize, then start donky. */
        initialize_stuff();
        //free_your_mind();
        
        clean_up_everything();

        exit(EXIT_SUCCESS);
}
/**
 * @brief Initialize everything donky needs to next begin drawing.
 */
void initialize_stuff(void)
{
        struct x_connection *x_conn;
        struct window_settings *ws;

        x_conn = init_x_connection();
        printf("made X connection\n");

        parse_cfg();
        printf("parsed config\n");

        parse_text();
        printf("parsed [text]\n");

        module_load_all();
        printf("loaded modules\n");

        ws = draw_window(x_conn);
        printf("drew window\n");

        donky_loop(x_conn, ws);
}

/**
 * @brief Run all cleanup methods for (each) source file.
 */
void clean_up_everything(void)
{
        clear_cfg();
        clear_text();
        clear_module();
        //clear_x();
}
