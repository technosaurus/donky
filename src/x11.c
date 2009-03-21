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

//include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>
#include <unistd.h>

#include "x11.h"
#include "config.h"
#include "render.h"
#include "text.h"
#include "module.h"
#include "util.h"

/** 
 * @brief Connects to the X server and stores all relevant information.
 *        Most things set in here are declared in the header.
 */
void init_x_connection(void)
{
        /* connect to X server */
        connection = xcb_connect(NULL, &screen_number);

        if (!connection) {
                printf("Can't connect to X server.\n");
                exit(EXIT_FAILURE);
        }

        setup = xcb_get_setup(connection);

        screen = NULL;
        
        screen_iter = xcb_setup_roots_iterator(setup);

        /* find our current screen */        
        for (; screen_iter.rem != 0; screen_number--, xcb_screen_next(&screen_iter))
                if (screen_number == 0) {
                        screen = screen_iter.data;
                        break;
                }

        /* exit if we can't find it */
        if (!screen) {
                printf("Can't find the current screen.\n");
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }
}

/** 
 * @brief Draw donky's window
 */
void draw_window(void)
{
        xcb_void_cookie_t window_cookie;
        uint32_t mask;
        uint32_t values[3];

        xcb_void_cookie_t map_cookie;
        
        char *X11mod = "X11";

        unsigned int d_width, d_height;  

        x_offset = get_int_key(X11mod, "XOffset");
        y_offset = get_int_key(X11mod, "YOffset");
        
        if (x_offset == -1) {
                x_offset = 0;
                printf("Didn't find x offset\n");
        }
        if (y_offset == -1) {
                y_offset = 0;
                printf("Didn't find y offset\n");
        }

        d_width = get_int_key(X11mod, "BarWidth");
        d_height = get_int_key(X11mod, "BarHeight");
        
        if (d_width == -1) {
                d_width = 1024;
                printf("Didn't find BarWidth\n");
        }
        if (d_height == -1) {
                d_height = 16;
                printf("Didn't find BarHeight\n");
        }

        /* TODO - learn how to do colors from cfg */
        bg_color = screen->white_pixel;
        fg_color = screen->black_pixel;

        window = xcb_generate_id(connection);
        mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
        values[0] = bg_color;
        values[1] = 1;
        values[2] = XCB_EVENT_MASK_EXPOSURE;
        window_cookie = xcb_create_window(
                        connection,
                        screen->root_depth,
                        window,
                        screen->root,
                        x_offset, y_offset,
                        d_width, d_height,
                        0,
                        XCB_WINDOW_CLASS_INPUT_OUTPUT, /* TODO: learn */
                        screen->root_visual,
                        mask, values);
        map_cookie = xcb_map_window_checked(connection, window);

        /* some error management */
        error = xcb_request_check(connection, window_cookie);
        if (error) {
                printf("Can't create window. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }
        
        error = xcb_request_check(connection, map_cookie);
        if (error) {
                printf("Can't map window. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }
}


/** 
 * @brief Main donky loop.
 */
void donky_loop(void)
{
        char *font_name = "fixed";
        xcb_font_t font;

        int offset;

        char *str;
        char *(*sym)(char *);
        struct module_var *mod;

        struct text_section *cur;
        struct text_section *next;
        
        cur = ts_start;
        cur->xpos = 3;
        cur->ypos = 10;

        while (1) {
                font = get_font(font_name);
                
                while (cur) {
                        next = cur->next;
                        printf("xpos = %d\n", cur->xpos);

                        switch (cur->type) {
                                case TEXT_FONT:
                                        close_font(font);
                                        font = get_font(cur->value);
                                        break;
                                case TEXT_COLOR:
                                        break;
                                case TEXT_STATIC:
                                        printf("got into TEXT_STATIC\n");
                                        offset = render_text(cur->value, font, cur->xpos, cur->ypos);
                                        printf("made it past offset = %d\n", offset);
                                        break;
                                case TEXT_VARIABLE:
                                        mod = module_var_find(cur->value);
                                        if (mod != NULL) {
                                                if ((get_time() - mod->last_update) < mod->timeout) {
                                                        printf("WAITING...\n");
                                                        /* save old x offset */
                                                        offset = cur->xpos;
                                                        break;
                                                }

                                                mod->last_update = get_time();
                                                printf("updating... %s\n", mod->name);
                                                sym = mod->sym;
                                                switch (mod->type) {
                                                        case VARIABLE_STR:
                                                                str = sym(cur->args);
                                                                offset = render_text(str, font, cur->xpos, cur->ypos);
                                                }
                                        }
                                        break;
                                default:
                                        printf("incorrect text_section type\n");
                                        break;
                        }

                        /* if we have a following node, we need 
                           to know where to start drawing it */
                        if (next) {
                                next->xpos = cur->xpos + offset;
                                next->ypos = cur->ypos;
                        }

                        cur = cur->next;
                        printf("set cur->next\n");
                }

                /* close font & flush everything to X server */
                close_font(font);
                xcb_flush(connection);

                cur = ts_start;
                printf("sleeping...\n");
                sleep(1);
        }
}

