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

#define _GNU_SOURCE
#include <string.h>
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
#include "default_settings.h"

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
        uint32_t window_bgcolor, window_fgcolor;
        int window_width, window_height;
        
        int x_gap, y_gap;
        int x_offset, y_offset;

        char *alignment = get_char_key("X11", "alignment");;

        xcb_void_cookie_t window_cookie;
        uint32_t mask;
        uint32_t values[3];

        xcb_void_cookie_t map_cookie;
        
        /* TODO - learn how to do colors from cfg */
        window_bgcolor = screen->black_pixel;
        window_fgcolor = screen->white_pixel;

        /* get window dimensions from cfg or set to defaults */
        window_width = get_int_key("X11", "window_width");
        window_height = get_int_key("X11", "window_height");

        if (window_width <= 0)
                window_width = DEFAULT_WINDOW_WIDTH;
        if (window_height <= 0)
                window_height = DEFAULT_WINDOW_HEIGHT;

        /* get gaps from cfg or set to defaults */
        x_gap = get_int_key("X11", "x_gap");
        y_gap = get_int_key("X11", "y_gap");

        if (x_gap < 0)
                x_gap = DEFAULT_X_GAP;
        if (y_gap < 0)
                y_gap = DEFAULT_Y_GAP;

        /* calculate alignment */
        if (alignment && (strcasecmp(alignment, "bottom_left") == 0)) {
                x_offset = 0 + x_gap;
                y_offset = screen->height_in_pixels - window_height - y_gap;
        }
        else if (alignment && (strcasecmp(alignment, "top_left") == 0)) {
                x_offset = 0 + x_gap;
                y_offset = 0 + y_gap;
        }
        else if (alignment && (strcasecmp(alignment, "bottom_right") == 0)) {
                x_offset = screen->width_in_pixels - window_width - x_gap;
                y_offset = screen->height_in_pixels - window_height - y_gap;
        }
        else if (alignment && (strcasecmp(alignment, "top_right") == 0)) {
                x_offset = screen->width_in_pixels - window_width - x_gap;
                y_offset = 0 + y_gap;
        }
        else {
                if (alignment)
                        printf("Unrecognized alignment: %s. Using bottom_left.\n", alignment);
                else
                        printf("No alignment specified. Using bottom_left.\n");
                x_offset = 0 + x_gap;
                y_offset = screen->height_in_pixels - window_height - y_gap;
        }

        window = xcb_generate_id(connection);
        mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
        values[0] = window_bgcolor;
        values[1] = 1;
        values[2] = XCB_EVENT_MASK_EXPOSURE;
        window_cookie = xcb_create_window(
                        connection,
                        screen->root_depth,
                        window,
                        screen->root,
                        x_offset, y_offset,
                        window_width, window_height,
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
        xcb_font_t font;
        xcb_query_text_extents_reply_t *extents;

        char *font_name = get_char_key("X11", "default_font");
        int free_font = 0;

        if (font_name == NULL) {
                font_name = strndup(DEFAULT_FONT, (sizeof(char) * strlen(DEFAULT_FONT)));
                free_font = 1;
        }

        int offset;

        char *str;
        char *(*sym)(char *);
        struct module_var *mod;

        struct text_section *cur;
        cur = ts_start;

        cur->xpos = get_int_key("X11", "font_x_offset");
        cur->ypos = get_int_key("X11", "font_y_offset");

        if (cur->xpos < 0)
                cur->xpos = 0;
        if (cur->ypos < 0)
                cur->ypos = 0;

        while (1) {
                font = get_font(font_name);
                
                while (cur) {
                        offset = 0;
                        printf("xpos = %d\n", cur->xpos);

                        switch (cur->type) {
                                case TEXT_FONT:
                                        close_font(font);
                                        font = get_font(cur->value);
                                        break;
                                case TEXT_COLOR:
                                        break;
                                case TEXT_STATIC:
                                        extents = get_extents(cur->value, font);
                                        offset = extents->overall_width;
                                        //if (prev->xpos && (cur->xpos != (prev->xpos + offset))) {
                                                render_text(cur->value, font, cur->xpos, cur->ypos);
                                                cur->pixel_width = offset;
                                        //}
                                        break;
                                case TEXT_VARIABLE:
                                        mod = module_var_find(cur->value);
                                        if (mod != NULL) {
                                                if ((get_time() - mod->last_update) < mod->timeout) {
                                                        printf("WAITING... %s\n", mod->name);
                                                        /* save old x offset */
                                                        offset = cur->pixel_width;
                                                        break;
                                                }

                                                mod->last_update = get_time();
                                                printf("updating... %s\n", mod->name);
                                                sym = mod->sym;
                                                switch (mod->type) {
                                                        case VARIABLE_STR:
                                                                str = sym(cur->args);
                                                                extents = get_extents(str, font);
                                                                offset = extents->overall_width;
                                                                render_text(str, font, cur->xpos, cur->ypos);
                                                                cur->pixel_width = offset;
                                                }
                                        }
                                        break;
                                default:
                                        printf("incorrect text_section type\n");
                                        break;
                        }

                        /* if we have a following node, we need 
                           to know where to start drawing it */
                        if (cur->next) {
                                cur->next->xpos = cur->xpos + cur->pixel_width;
                                cur->next->ypos = cur->ypos;
                        }

                        cur = cur->next;
                }

                /* close font & flush everything to X server */
                free(extents);
                close_font(font);
                xcb_flush(connection);

                cur = ts_start;
                sleep(1);
        }
}

