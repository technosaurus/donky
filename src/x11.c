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
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
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
int own_window;
int x_offset;
int y_offset;

/** 
 * @brief Connects to the X server and stores all relevant information.
 *        Most things set in here are declared in the header.
 */
void init_x_connection(struct x_connection *x_conn)
{
        const xcb_setup_t *setup;
        xcb_screen_iterator_t screen_iter;
        int screen_number;

        /* connect to X server */
        x_conn->display = XOpenDisplay(NULL);
        x_conn->connection = XGetXCBConnection(x_conn->display);

        if (!x_conn->connection) {
                printf("Can't connect to X server.\n");
                exit(EXIT_FAILURE);
        }

        setup = xcb_get_setup(x_conn->connection);

        x_conn->screen = NULL;
        
        screen_iter = xcb_setup_roots_iterator(setup);

        /* find our current screen */        
        for (; screen_iter.rem != 0; screen_number--, xcb_screen_next(&screen_iter)) {
                if (screen_number == 0) {
                        x_conn->screen = screen_iter.data;
                        break;
                }
        }

        /* exit if we can't find it */
        if (!x_conn->screen) {
                printf("Can't find the current screen.\n");
                xcb_disconnect(x_conn->connection);
                exit(EXIT_FAILURE);
        }
}

/** 
 * @brief Draw donky's window
 */
void draw_window(struct x_connection *x_conn)
{
        uint32_t window_bgcolor;
        uint32_t window_fgcolor;
        char *window_bgcolor_name;
        char *window_fgcolor_name;

        int override;
        
        int x_gap;
        int y_gap;

        char *alignment = get_char_key("X11", "alignment");

        xcb_void_cookie_t window_cookie;
        uint32_t mask;
        uint32_t values[3];

        xcb_void_cookie_t map_cookie;

        xcb_generic_error_t *error;

        /* Set up window bg and fg colors. */
        window_bgcolor_name = get_char_key("X11", "window_bgcolor");
        window_fgcolor_name = get_char_key("X11", "window_fgcolor");

        if (window_bgcolor_name == NULL)
                window_bgcolor_name = d_strcpy(DEFAULT_WINDOW_BGCOLOR);
        if (window_fgcolor_name == NULL)
                window_fgcolor_name = d_strcpy(DEFAULT_WINDOW_FGCOLOR);
                
        window_bgcolor = get_color(x_conn->connection,
                                   x_conn->screen,
                                   window_bgcolor_name);
        window_fgcolor = get_color(x_conn->connection,
                                   x_conn->screen,
                                   window_fgcolor_name);

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
                y_offset = x_conn->screen->height_in_pixels - window_height - y_gap;
        } else if (alignment && (strcasecmp(alignment, "top_left") == 0)) {
                x_offset = 0 + x_gap;
                y_offset = 0 + y_gap;
        } else if (alignment && (strcasecmp(alignment, "bottom_right") == 0)) {
                x_offset = x_conn->screen->width_in_pixels - window_width - x_gap;
                y_offset = x_conn->screen->height_in_pixels - window_height - y_gap;
        } else if (alignment && (strcasecmp(alignment, "top_right") == 0)) {
                x_offset = x_conn->screen->width_in_pixels - window_width - x_gap;
                y_offset = 0 + y_gap;
        } else {
                if (alignment)
                        printf("Unrecognized alignment: %s. Using bottom_left.\n", alignment);
                else
                        printf("No alignment specified. Using bottom_left.\n");
                x_offset = 0 + x_gap;
                y_offset = x_conn->screen->height_in_pixels - window_height - y_gap;
        }

        freeif(alignment);

        /* draw donky in its own window? if not, draw to root */
        own_window = get_bool_key("X11", "own_window");
        if (own_window <= 0) {
                own_window = 0;
                x_conn->window = x_conn->screen->root;
                return;
        }
        /* if it's not 0 or -1, it's 1 */

        /* check if donky's window should override wm control */
        override = get_bool_key("X11", "override");
        if (override == -1)
                override = 0;

        x_conn->window = xcb_generate_id(x_conn->connection);
        mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
        values[0] = window_bgcolor;
        values[1] = override;
        values[2] = XCB_EVENT_MASK_EXPOSURE;
        window_cookie = xcb_create_window(
                        x_conn->connection,
                        x_conn->screen->root_depth,
                        x_conn->window,
                        x_conn->screen->root,
                        x_offset, y_offset,
                        window_width, window_height,
                        0,
                        XCB_WINDOW_CLASS_INPUT_OUTPUT, /* TODO: learn */
                        x_conn->screen->root_visual,
                        mask, values);
        map_cookie = xcb_map_window_checked(x_conn->connection,
                                            x_conn->window);
        /* some error management */
        error = xcb_request_check(x_conn->connection, window_cookie);
        if (error) {
                printf("Can't create window. Error: %d\n", error->error_code);
                xcb_disconnect(x_conn->connection);
                exit(EXIT_FAILURE);
        }
        
        error = xcb_request_check(x_conn->connection, map_cookie);
        if (error) {
                printf("Can't map window. Error: %d\n", error->error_code);
                xcb_disconnect(x_conn->connection);
                exit(EXIT_FAILURE);
        }
}

/**
 * @brief Load all the donky draw settings.
 *
 * @return Malloc'd donky_draw_settings structure
 */
struct donky_draw_settings *donky_draw_settings_load(struct x_connection *x_conn)
{
        struct donky_draw_settings *dds = malloc(sizeof(struct donky_draw_settings));

        /* Set up user configured font or default font. */
        dds->font_name = get_char_key("X11", "default_font");

        if (dds->font_name == NULL)
                dds->font_name = d_strcpy(DEFAULT_FONT);

        dds->font_struct = XLoadQueryFont(x_conn->display, dds->font_name);
        dds->font = dds->font_struct->fid;

        /* Set up user configured or default font colors. */
        dds->color_name_bg = get_char_key("X11", "font_bgcolor");
        dds->color_name_fg = get_char_key("X11", "font_fgcolor");

        if (dds->color_name_bg == NULL)
                dds->color_name_bg = d_strcpy(DEFAULT_FONT_BGCOLOR);
        if (dds->color_name_bg == NULL)
                dds->color_name_fg = d_strcpy(DEFAULT_FONT_FGCOLOR);

        dds->color_bg_orig = get_color(x_conn->connection,
                                  x_conn->screen,
                                  dds->color_name_bg);
        dds->color_fg_orig = get_color(x_conn->connection,
                                  x_conn->screen,
                                  dds->color_name_fg);

        /* Setup the position of the starting text section. */
        dds->font_x_offset = get_int_key("X11", "font_x_offset");
        dds->font_y_offset = get_int_key("X11", "font_y_offset");

        if (dds->font_x_offset < 0)
                dds->font_x_offset = 0;
        if (dds->font_y_offset < 0)
                dds->font_y_offset = 0;

        /* for alignment to work on root drawing */
        if (own_window == 0) {
                dds->font_x_offset += x_offset;
                dds->font_y_offset += y_offset;
        }

        /* Setup minimum sleep time. */
        double min_sleep = get_double_key("X11", "global_sleep");
        
        if (min_sleep < 0)
                min_sleep = 1.0;
                
        int min_seconds = floor(min_sleep);
        long min_nanosec = (min_sleep - min_seconds) * pow(10, 9);
        dds->tspec.tv_sec = min_seconds;
        dds->tspec.tv_nsec = min_nanosec;

        return dds;
}

/** 
 * @brief Main donky loop.
 */
void donky_loop(struct x_connection *x_conn)
{
        int is_last;

        char *str;
        char *(*sym)(char *);
        struct module_var *mod;

        int16_t new_xpos;
        int16_t new_ypos;

        xcb_generic_event_t *e;
        uint8_t force = 0;

        struct text_section *cur = ts_start;
        struct donky_draw_settings *dds = donky_draw_settings_load(x_conn);

        //xcb_generic_event_t *event;

        /* new_xpos and new_ypos hold the coords
         * of where we'll be drawing anything new */
        new_xpos = dds->font_x_offset;
        new_ypos = dds->font_y_offset;

        /* Infinite donky loop! (TM) :o */
        while (1) {

                while (e = xcb_poll_for_event(x_conn->connection)) {
                        switch (e->response_type & ~0x80) {
                                case XCB_EXPOSE:
                                        force = 1;
                                        break;
                                default:
                                        break;
                        }
                        free(e);
                }

                dds->color.pixel_bg = dds->color_bg_orig;
                dds->color.pixel_fg = dds->color_fg_orig;

                /* Execute cron tabs. */
                module_var_cron_exec();
                
                while (cur) {
                        is_last = 0;

                        if ((cur->next && cur->line != cur->next->line) ||
                            cur->next == NULL)
                            is_last = 1;

                        switch (cur->type) {
                        case TEXT_FONT:
                                /*if (font)
                                        close_font(font);

                                if (cur->args == NULL)
                                        font = font_orig;
                                else if (!strcasecmp(cur->args, font_name))
                                        font = font_orig;
                                else
                                        font = get_font(cur->args);
                                break;*/
                        case TEXT_COLOR:
                                if (cur->args == NULL)
                                        dds->color.pixel_fg = get_color(x_conn->connection,
                                                                        x_conn->screen,
                                                                        dds->color_name_fg);
                                else
                                        dds->color.pixel_fg = get_color(x_conn->connection,
                                                                        x_conn->screen,
                                                                        cur->args);
                                break;
                        case TEXT_STATIC:
                                /* If the xpos has changed since last drawn,
                                 * we will need to redraw this static text.
                                 * Otherwise, we never redraw it! */
                                if (cur->xpos == -1 || (cur->xpos != new_xpos) || force) {
                                        //printf("REDRAWING STATIC TEXT [%s]\n", cur->value);
                                        if (!cur->pixel_width)
                                                cur->pixel_width = XTextWidth(dds->font_struct,
                                                                              cur->value,
                                                                              strlen(cur->value));

                                        cur->xpos = new_xpos;
                                        cur->ypos = new_ypos;

                                        render_queue_add(cur->value,
                                                         dds->color,
                                                         dds->font,
                                                         &cur->xpos,
                                                         &cur->ypos,
                                                         cur->xpos,
                                                         cur->ypos - dds->font_y_offset,
                                                         cur->pixel_width,
                                                         window_height,
                                                         is_last);
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
                                if (!force &&
                                    (((get_time() - cur->last_update < cur->timeout) ||
                                      (cur->timeout == 0 && (cur->last_update != 0))) &&
                                     (cur->xpos == new_xpos))) {
                                        //printf("WAITING... %s\n", mod->name);
                                        break;
                                }

                                cur->xpos = new_xpos;
                                cur->ypos = new_ypos;

                                sym = mod->sym;
                                switch (mod->type) {
                                case VARIABLE_STR:
                                        if ((get_time() - cur->last_update >= cur->timeout) ||
                                            (cur->timeout == 0 && (cur->last_update == 0))) {
                                                strncpy(cur->result,
                                                        sym(cur->args),
                                                        sizeof(cur->result));

                                                cur->last_update = get_time();
                                                //printf("Updating... %s\n", mod->name);
                                        }

                                        cur->pixel_width = XTextWidth(dds->font_struct,
                                                                      cur->result,
                                                                      strlen(cur->result));

                                        render_queue_add(cur->result,
                                                         dds->color,
                                                         dds->font,
                                                         &cur->xpos,
                                                         &cur->ypos,
                                                         cur->xpos,
                                                         cur->ypos - dds->font_y_offset,
                                                         cur->pixel_width,
                                                         window_height,
                                                         is_last);
                                                    
                                        break;
                                case VARIABLE_BAR:
                                        break;
                                case VARIABLE_GRAPH:
                                        break;
                                case VARIABLE_CUSTOM:
                                        break;
                                default:
                                        printf("Invalid module variable_type.\n");
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
                                new_xpos += cur->pixel_width;
                                //new_ypos += TODO
                        } else {
                                new_xpos = dds->font_x_offset;
                                new_ypos = dds->font_y_offset;
                        }

                        /* Next node... */
                        cur = cur->next;
                }

                /* Render everything. */
                render_queue_exec(x_conn->connection,
                                  &x_conn->window);

                /* Flush XCB like a friggin' toilet. */
                xcb_flush(x_conn->connection);

                /* Clear mem list... */
                mem_list_clear();

                /* Reset cur and take a nap. */
                cur = ts_start;

                /* Sleep set amount of seconds and nanoseconds.  nanosleep
                 * returns -1 if it detects a signal hander being invoked, so
                 * we opt to break from the loop since it will probably be the
                 * reload or kill handler. */
                if (nanosleep(&dds->tspec, NULL) == -1)
                        break;
        }

        if (force == 1)
                force = 0;

        freeif(dds->font_name);
        freeif(dds->color_name_bg);
        freeif(dds->color_name_fg);
        freeif(dds);
        /* cleaning Xlib shiz up here too */
}

