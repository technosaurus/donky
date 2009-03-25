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
#include <time.h>
#include <math.h>

#include "x11.h"
#include "config.h"
#include "render.h"
#include "text.h"
#include "module.h"
#include "util.h"
#include "default_settings.h"
#include "mem.h"

/* Globals. */
int window_width;
int window_height;
int own_window;
int x_offset;
int y_offset;

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
        uint32_t window_bgcolor;
        uint32_t window_fgcolor;
        char *window_bgcolor_name;
        char *window_fgcolor_name;

        int override;
        
        int x_gap;
        int y_gap;

        char *alignment = get_char_key("X11", "alignment");;

        xcb_void_cookie_t window_cookie;
        uint32_t mask;
        uint32_t values[3];

        xcb_void_cookie_t map_cookie;
        
        /* Set up window bg and fg colors. */
        window_bgcolor_name = get_char_key("X11", "window_bgcolor");
        window_fgcolor_name = get_char_key("X11", "window_fgcolor");

        if (window_bgcolor_name == NULL)
                window_bgcolor_name = strdup(DEFAULT_WINDOW_BGCOLOR);
        if (window_fgcolor_name == NULL)
                window_fgcolor_name = strdup(DEFAULT_WINDOW_FGCOLOR);
                
        window_bgcolor = get_color(window_bgcolor_name);
        window_fgcolor = get_color(window_fgcolor_name);

        freeif(window_bgcolor_name);
        freeif(window_fgcolor_name);

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

        freeif(alignment);

        /* draw donky in its own window? if not, draw to root */
        own_window = get_bool_key("X11", "own_window");
        if (own_window <= 0) {
                own_window = 0;
                window = screen->root;
                return;
        }
        /* if it's not 0 or -1, it's 1 */

        /* check if donky's window should override wm control */
        override = get_bool_key("X11", "override");
        if (override == -1)
                override = 0;

        window = xcb_generate_id(connection);
        mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
        values[0] = window_bgcolor;
        values[1] = override;
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
        char *font_name;
        xcb_query_text_extents_reply_t *extents;
        
        struct donky_color color;
        char *color_name_bg;
        char *color_name_fg;

        uint16_t offset;
        
        int16_t font_x_offset;
        int16_t font_y_offset;

        char *str;
        char *(*sym)(char *);
        struct module_var *mod;

        struct text_section *cur;

        /* Set up user configured font or default font. */
        font_name = get_char_key("X11", "default_font");

        if (font_name == NULL)
                font_name = strndup(DEFAULT_FONT, (sizeof(char) * strlen(DEFAULT_FONT)));

        font_orig = get_font(font_name);

        /* Set up user configured or default font colors. */
        color_name_bg = get_char_key("X11", "font_bgcolor");
        color_name_fg = get_char_key("X11", "font_fgcolor");

        if (color_name_bg == NULL)
                color_name_bg = strdup(DEFAULT_FONT_BGCOLOR);
        if (color_name_bg == NULL)
                color_name_fg = strdup(DEFAULT_FONT_FGCOLOR);

        color_bg_orig = get_color(color_name_bg);
        color_fg_orig = get_color(color_name_fg);

        /* Setup the position of the starting text section. */
        cur = ts_start;
        font_x_offset = get_int_key("X11", "font_x_offset");
        font_y_offset = get_int_key("X11", "font_y_offset");

        if (font_x_offset < 0)
                font_x_offset = 0;
        if (font_y_offset < 0)
                font_y_offset = 0;

        cur->xpos = font_x_offset;
        cur->ypos = font_y_offset;

        /* for alignment to work on root drawing */
        if (own_window == 0) {
                cur->xpos += x_offset;
                cur->ypos += y_offset;
        }

        /* Setup minimum sleep time. */
        struct timespec tspec;
        double min_sleep = get_double_key("X11", "global_sleep");
        
        if (min_sleep < 0)
                min_sleep = 1.0;
                
        int min_seconds = floor(min_sleep);
        long min_nanosec = (min_sleep - min_seconds) * pow(10, 9);
        tspec.tv_sec = min_seconds;
        tspec.tv_nsec = min_nanosec;

        /* Infinite donky loop! (TM) :o */
        while (1) {
                font = font_orig;
                color.pixel_bg = color_bg_orig;
                color.pixel_fg = color_fg_orig;

                /* Execute cron tabs. */
                module_var_cron_exec();
                
                while (cur) {
                        extents = NULL;
                        offset = 0;
                        //printf("X pos = %d\n", cur->xpos);

                        /* If our X position has changed, lets clear the area
                         * where we used to be. */
                        if (cur->xpos != cur->old_xpos)
                                clear_queue_add(cur->old_xpos,
                                                cur->old_ypos - font_y_offset,
                                                cur->old_pixel_width,
                                                window_height);

                        switch (cur->type) {
                        case TEXT_FONT:
                                //if (font != font_orig)
                                        //close_font(font);

                                /*if (cur->args == NULL)
                                        font = font_orig;
                                else if (!strcasecmp(cur->args, font_name))
                                        font = font_orig;
                                else
                                        font = get_font(cur->args);*/
                                break;
                        case TEXT_COLOR:
                                if (cur->args == NULL)
                                        color.pixel_fg = get_color(color_name_fg);
                                else
                                        color.pixel_fg = get_color(cur->args);
                                break;
                        case TEXT_STATIC:
                                /* If the xpos has changed since last drawn,
                                 * we will need to redraw this static text.
                                 * Otherwise, we never redraw it! */
                                if (cur->old_xpos == -1 || cur->old_xpos != cur->xpos) {
                                        //printf("REDRAWING STATIC TEXT [%s]\n", cur->value);
                                        if (!cur->pixel_width) {
                                                extents = get_extents(cur->value, font);
                                                offset = extents->overall_width;
                                                if (!cur->pixel_width && (!cur->old_pixel_width))
                                                        cur->pixel_width = cur->old_pixel_width = offset;
                                        } else
                                                offset = cur->pixel_width;

                                        render_queue_add(cur->value,
                                                         color,
                                                         font,
                                                         &cur->xpos,
                                                         &cur->ypos);
                                        /*
                                        render_text(cur->value,
                                                    font,
                                                    color,
                                                    cur->xpos,
                                                    cur->ypos);*/
                                }
                                break;
                        case TEXT_VARIABLE:
                                mod = cur->mod_var;
                                if (mod == NULL)
                                        break;

                                /* We only update the value of this variable
                                 * if it has timed out or if timeout is set to
                                 * 0, meaning never update.
                                 * 
                                 * NOTE: we should update if our
                                 * xpos has changed! */
                                if (((get_time() - cur->last_update < cur->timeout) ||
                                    (cur->timeout == 0 && cur->last_update != 0)) &&
                                    (cur->old_xpos == cur->xpos)) {
                                        //printf("WAITING... %s\n", mod->name);
                                        /* save old x offset */
                                        offset = cur->pixel_width;
                                        break;
                                }

                                cur->last_update = get_time();
                                //printf("Updating... %s\n", mod->name);
                                sym = mod->sym;
                                switch (mod->type) {
                                case VARIABLE_STR:
                                        str = sym(cur->args);
                                        extents = get_extents(str, font);
                                        offset = extents->overall_width;
                                        
                                        /*render_text(str,
                                                    font,
                                                    color,
                                                    cur->xpos,
                                                    cur->ypos);*/
                                        render_queue_add(str,
                                                         color,
                                                         font,
                                                         &cur->xpos,
                                                         &cur->ypos);
                                                    
                                        if (cur->pixel_width)
                                                cur->old_pixel_width = cur->pixel_width;
                                        cur->pixel_width = offset;
                                        break;
                                case VARIABLE_BAR:
                                        break;
                                case VARIABLE_GRAPH:
                                        break;
                                case VARIABLE_CUSTOM:
                                        break;
                                default:
                                        printf("Invalid module variable_type\n");
                                        break;
                                }
                                break;
                        default:
                                printf("Invalid text_section type.\n");
                                break;
                        }

                        /* if we have a following node, we need 
                           to know where to start drawing it */
                        if (cur->next) {
                                cur->next->xpos = cur->xpos + cur->pixel_width;
                                cur->next->ypos = cur->ypos;
                        }

                        /* Set our current X and Y pos as old. */
                        cur->old_xpos = cur->xpos;
                        cur->old_ypos = cur->ypos;

                        /* Make sure we free this mother,
                         * was leaking on me earlier! */
                        freeif(extents);

                        /* Next node... */
                        cur = cur->next;
                }

                /* Render everything and clear mem list... */
                clear_queue_exec();
                render_queue_exec();
                mem_list_clear();

                /* Flush XCB like a friggin' toilet. */
                xcb_flush(connection);

                /* Reset cur and take a nap. */
                cur = ts_start;

                /* Sleep set amount of seconds and nanoseconds.  nanosleep
                 * returns -1 if it detects a signal hander being invoked, so
                 * we opt to break from the loop since it will probably be the
                 * reload or kill handler. */
                if (nanosleep(&tspec, NULL) == -1)
                        break;
        }

        close_font(font_orig);
        freeif(font_name);
        freeif(color_name_bg);
        freeif(color_name_fg);
}

