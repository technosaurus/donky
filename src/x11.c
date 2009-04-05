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
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <xcb/xcb.h>

#include "config.h"
#include "default_settings.h"
#include "mem.h"
#include "module.h"
#include "render.h"
#include "text.h"
#include "util.h"
#include "x11.h"

struct draw_settings {
        xcb_font_t font;
        XFontStruct *font_struct;
        char *font_name;

        int16_t font_x_offset;
        int16_t font_y_offset;

        int16_t min_line_height;
        int16_t min_line_spacing;

        struct donky_color color;

        struct timespec tspec;
};

/* Function prototypes. */
struct window_settings *window_settings_load(struct x_connection *x_conn);
struct draw_settings *draw_settings_load(struct x_connection *x_conn,
                                         struct window_settings *ws);
void handle_TEXT_COLOR(struct x_connection *x_conn,
                       struct text_section *cur,
                       struct draw_settings *ds);
void handle_TEXT_STATIC(struct text_section *cur,
                        struct draw_settings *ds,
                        int16_t *new_xpos,
                        int16_t *new_ypos,
                        unsigned int *force,
                        int *line_heights,
                        int *calcd_line_heights,
                        unsigned int *is_last);
void handle_TEXT_VARIABLE(struct text_section *cur,
                          struct draw_settings *ds,
                          int16_t *new_xpos,
                          int16_t *new_ypos,
                          unsigned int *force,
                          int *line_heights,
                          int *calcd_line_heights,
                          unsigned int *is_last);
void handle_VARIABLE_STR(struct text_section *cur,
                         struct draw_settings *ds,
                         int *line_heights,
                         int *calcd_line_heights,
                         unsigned int *is_last);
void handle_VARIABLE_BAR(struct text_section *cur,
                         struct draw_settings *ds,
                         int *line_heights,
                         int *calcd_line_heights,
                         unsigned int *is_last);
void clean_x(struct x_connection *x_conn,
             struct window_settings *ws,
             struct draw_settings *ds,
             int *line_heights);


/* Globals. */
extern int donky_reload;
extern int donky_exit;

/**
 * @brief Connects to the X server and stores all relevant information.
 *        Most things set in here are declared in the header.
 */
struct x_connection *init_x_connection(void)
{
        struct x_connection *x_conn;
        x_conn = malloc(sizeof(struct x_connection));

        xcb_screen_iterator_t screen_iter;
        int screen_number;

        /* connect to X server */
        x_conn->display = XOpenDisplay(NULL);
        x_conn->connection = XGetXCBConnection(x_conn->display);

        if (!x_conn->connection) {
                printf("Can't connect to X server.\n");
                exit(EXIT_FAILURE);
        }

        x_conn->screen = NULL;

        screen_iter = xcb_setup_roots_iterator(xcb_get_setup(x_conn->connection));

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

        return x_conn;
}

/**
 * @brief Gather and store necessary information for donky's
 *        to-be-drawn window.
 *
 * @param x_conn donky's open X connection.
 *
 * @return malloc'd & filled windows_settings struct
 */
struct window_settings *window_settings_load(struct x_connection *x_conn)
{
        char *bg_color_name;
        char *fg_color_name;

        char *alignment;
        uint16_t x_gap;
        uint16_t y_gap;

        struct window_settings *ws;
        ws = malloc(sizeof(struct window_settings));

        /* Set up window bg and fg colors. */
        bg_color_name = get_char_key("X11", "window_bgcolor");
        fg_color_name = get_char_key("X11", "window_fgcolor");

        if (bg_color_name == NULL)
                bg_color_name = d_strcpy(DEFAULT_WINDOW_BGCOLOR);
        if (fg_color_name == NULL)
                fg_color_name = d_strcpy(DEFAULT_WINDOW_FGCOLOR);

        ws->bg_color = get_color(x_conn->connection,
                                 x_conn->screen,
                                 bg_color_name);
        ws->fg_color = get_color(x_conn->connection,
                                 x_conn->screen,
                                 fg_color_name);

        free(bg_color_name);
        free(fg_color_name);

        /* get window dimensions from cfg or set to defaults */
        ws->width = get_int_key("X11", "window_width");
        ws->height = get_int_key("X11", "window_height");

        if (ws->width <= 0)
                ws->width = DEFAULT_WINDOW_WIDTH;
        if (ws->height <= 0)
                ws->height = DEFAULT_WINDOW_HEIGHT;

        /* get gaps from cfg or set to defaults */
        x_gap = get_int_key("X11", "x_gap");
        y_gap = get_int_key("X11", "y_gap");

        if (x_gap < 0)
                x_gap = DEFAULT_X_GAP;
        if (y_gap < 0)
                y_gap = DEFAULT_Y_GAP;

        alignment = get_char_key("X11", "alignment");

        /* calculate alignment */
        if (alignment && (strcasecmp(alignment, "bottom_left") == 0)) {
                ws->x_offset = 0 + x_gap;
                ws->y_offset = x_conn->screen->height_in_pixels - ws->height - y_gap;
        } else if (alignment && (strcasecmp(alignment, "top_left") == 0)) {
                ws->x_offset = 0 + x_gap;
                ws->y_offset = 0 + y_gap;
        } else if (alignment && (strcasecmp(alignment, "bottom_right") == 0)) {
                ws->x_offset = x_conn->screen->width_in_pixels - ws->width - x_gap;
                ws->y_offset = x_conn->screen->height_in_pixels - ws->height - y_gap;
        } else if (alignment && (strcasecmp(alignment, "top_right") == 0)) {
                ws->x_offset = x_conn->screen->width_in_pixels - ws->width - x_gap;
                ws->y_offset = 0 + y_gap;
        } else {
                if (alignment)
                        printf("Unrecognized alignment: %s. Using bottom_left.\n", alignment);
                else
                        printf("No alignment specified. Using bottom_left.\n");
                ws->x_offset = 0 + x_gap;
                ws->y_offset = x_conn->screen->height_in_pixels - ws->height - y_gap;
        }

        freeif(alignment);

        /* draw donky in its own window? if not, draw to root */
        ws->own_window = get_bool_key("X11", "own_window");
        if (ws->own_window == -1) {
                ws->own_window = 0;
                x_conn->window = x_conn->screen->root;
                return;
        }

        /* check if donky's window should override wm control */
        ws->override = get_bool_key("X11", "override");
        if (ws->override == -1)
                ws->override = 0;

        return ws;
}

/**
 * @brief Gather window settings (see window_settings_load())
 *        and draw window.
 *
 * @param x_conn donky's open X connection.
 *
 * @return The windows_setting struct used in drawing the window.
 */
struct window_settings *draw_window(struct x_connection *x_conn)
{
        struct window_settings *ws;
        ws = window_settings_load(x_conn);

        xcb_void_cookie_t window_cookie;
        uint32_t mask;
        uint32_t values[3];

        xcb_void_cookie_t map_cookie;

        xcb_generic_error_t *error;

        x_conn->window = xcb_generate_id(x_conn->connection);
        mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK;
        values[0] = ws->bg_color;
        values[1] = ws->override;
        values[2] = XCB_EVENT_MASK_EXPOSURE;
        window_cookie = xcb_create_window(
                        x_conn->connection,
                        x_conn->screen->root_depth,
                        x_conn->window,
                        x_conn->screen->root,
                        ws->x_offset, ws->y_offset,
                        ws->width, ws->height,
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

        return ws;
}

/**
 * @brief Load all the donky draw settings.
 *
 * @return Malloc'd draw_settings structure
 */
struct draw_settings *draw_settings_load(struct x_connection *x_conn,
                                         struct window_settings *ws)
{
        struct draw_settings *ds = malloc(sizeof(struct draw_settings));

        /* Set up user configured font or default font. */
        ds->font_name = get_char_key("X11", "default_font");

        if (ds->font_name == NULL)
                ds->font_name = d_strcpy(DEFAULT_FONT);

        ds->font_struct = XLoadQueryFont(x_conn->display, ds->font_name);
        ds->font = ds->font_struct->fid;

        /* Set up user configured or default font colors. */
        ds->color.bg_name = get_char_key("X11", "font_bgcolor");
        ds->color.fg_name = get_char_key("X11", "font_fgcolor");

        if (ds->color.bg_name == NULL)
                ds->color.bg_name = d_strcpy(DEFAULT_FONT_BGCOLOR);
        if (ds->color.bg_name == NULL)
                ds->color.fg_name = d_strcpy(DEFAULT_FONT_FGCOLOR);

        ds->color.bg_orig = get_color(x_conn->connection,
                                      x_conn->screen,
                                      ds->color.bg_name);
        ds->color.fg_orig = get_color(x_conn->connection,
                                      x_conn->screen,
                                      ds->color.fg_name);

        /* Setup the position of the starting text section. */
        ds->font_x_offset = get_int_key("X11", "font_x_offset");
        ds->font_y_offset = get_int_key("X11", "font_y_offset");

        if (ds->font_x_offset < 0)
                ds->font_x_offset = DEFAULT_FONT_X_OFFSET;
        if (ds->font_y_offset < 0)
                ds->font_y_offset = DEFAULT_FONT_Y_OFFSET;

        /* Setup minimum line height. */
        ds->min_line_height = get_int_key("X11", "minimum_line_height");

        if (ds->min_line_height < 0)
                ds->min_line_height = DEFAULT_MINIMUM_LINE_HEIGHT;

        /* Setup minimum line spacing. */
        ds->min_line_spacing = get_int_key("X11", "minimum_line_spacing");

        if (ds->min_line_spacing < 0)
                ds->min_line_spacing = DEFAULT_MINIMUM_LINE_SPACING;

        /* for alignment to work on root drawing */
        if (ws->own_window == 0) {
                ds->font_x_offset += ws->x_offset;
                ds->font_y_offset += ws->y_offset;
        }

        /* Setup minimum sleep time. */
        double min_sleep = get_double_key("X11", "global_sleep");

        if (min_sleep < 0)
                min_sleep = DEFAULT_GLOBAL_SLEEP;

        int min_seconds = floor(min_sleep);
        long min_nanosec = (min_sleep - min_seconds) * pow(10, 9);
        ds->tspec.tv_sec = min_seconds;
        ds->tspec.tv_nsec = min_nanosec;

        return ds;
}

/** 
 * @brief main donky loop
 * 
 * @param x_conn donky's open x_connection struct
 * @param ws donky's window_settings struct
 */
void donky_loop(struct x_connection *x_conn,
                struct window_settings *ws)
{
        struct text_section *cur = ts_start;
        struct draw_settings *ds = draw_settings_load(x_conn, ws);

        xcb_generic_event_t *e;
        unsigned int force = 0;

        /* new_xpos and new_ypos hold the coords
         * of where we'll be drawing anything new */
        int16_t new_xpos = ds->font_x_offset;
        int16_t new_ypos = ds->font_y_offset;

        /* Line height pewp. */
        int *line_heights = calloc(sizeof(int), ts_end->line + 1);
        int calcd_line_heights = 0;
        int linediff;
        int i;

        unsigned int is_last;

        /* Infinite donky loop! (TM) :o */
        while (!donky_reload && !donky_exit) {
                if (force)
                        force = 0;

                /* Do a quick event poll. */
                while (e = xcb_poll_for_event(x_conn->connection)) {
                        switch (e->response_type & ~0x80) {
                                case XCB_EXPOSE:
                                        /* force redraw everything */
                                        if (!force)
                                                force = 1;
                                        break;
                                default:
                                        break;
                        }
                        free(e);
                }

                ds->color.bg = ds->color.bg_orig;
                ds->color.fg = ds->color.fg_orig;

                /* Execute cron tabs. */
                module_var_cron_exec();

                while (cur) {
                        if ((cur->next == NULL) ||
                            (cur->line != cur->next->line))
                                is_last = 1;
                        else
                                is_last = 0;

                        switch (cur->type) {
                                case TEXT_FONT:
                                        break;
                                case TEXT_COLOR:
                                        handle_TEXT_COLOR(x_conn, cur, ds);
                                        break;
                                case TEXT_STATIC:
                                        handle_TEXT_STATIC(cur,
                                                           ds,
                                                           &new_xpos,
                                                           &new_ypos,
                                                           &force,
                                                           line_heights,
                                                           &calcd_line_heights,
                                                           &is_last);
                                        break;
                                case TEXT_VARIABLE:
                                        handle_TEXT_VARIABLE(cur,
                                                             ds,
                                                             &new_xpos,
                                                             &new_ypos,
                                                             &force,
                                                             line_heights,
                                                             &calcd_line_heights,
                                                             &is_last);
                                        break;
                                default:
                                        printf("Invalid text_section type.\n");
                                        break;
                        }

                        /* if we have a following node, we need
                           to know where to start drawing it */
                        if (cur->next) {
                                new_xpos += cur->pixel_width;

                                if (cur->line != cur->next->line) {
                                        /* Set xpos to beginning of line! */
                                        new_xpos = ds->font_x_offset;

                                        /* Calculate the difference in lines
                                         * (Needed for multiple blank lines
                                         *  between text.) */
                                        linediff =
                                                cur->next->line -
                                                cur->line - 1;

                                        if (line_heights[cur->line] >
                                            ds->min_line_height)
                                                new_ypos +=
                                                        line_heights[cur->line];
                                        else
                                                new_ypos +=
                                                        ds->min_line_height +
                                                        ds->min_line_spacing;

                                        for (i = 0; i < linediff; i++)
                                                new_ypos +=
                                                        ds->min_line_height +
                                                        ds->min_line_spacing;
                                }
                        } else {
                                new_xpos = ds->font_x_offset;
                                new_ypos = ds->font_y_offset;
                        }

                        /* Next node... */
                        cur = cur->next;
                }

                /* Render everything. */
                render_queue_exec(x_conn->connection,
                                  &x_conn->window,
                                  &ws->width);

                /* Flush XCB like a friggin' toilet. */
                xcb_flush(x_conn->connection);

                /* Clear mem list... */
                mem_list_clear();

                /* Reset cur. */
                cur = ts_start;

                /* Set a flag so we know we've
                 * already calc'd all line heights. */
                if (!calcd_line_heights)
                        calcd_line_heights = 1;

                /* Sleep set amount of seconds and nanoseconds.  nanosleep
                 * returns -1 if it detects a signal hander being invoked, so
                 * we opt to break from the loop since it will probably be the
                 * reload or kill handler. */
                if (nanosleep(&ds->tspec, NULL) == -1) {
                        printf("Breaking...\n");
                        break;
                }
        }

        clean_x(x_conn, ws, ds, line_heights);
}

/** 
 * @brief Handles font color changes.
 * 
 * @param x_conn donky's open X connection
 * @param cur Current text_section node
 * @param ds donky's draw settings
 */
void handle_TEXT_COLOR(struct x_connection *x_conn,
                       struct text_section *cur,
                       struct draw_settings *ds)
{
        if (cur->args == NULL)
                ds->color.fg = get_color(x_conn->connection,
                                         x_conn->screen,
                                         ds->color.fg_name);
        else
                ds->color.fg = get_color(x_conn->connection,
                                         x_conn->screen,
                                         cur->args);
}

/**
 * @brief Handles TEXT_STATIC text_section types.
 *        Takes care of drawing any static text.
 *
 * @param cur Current text_section node
 * @param ds donky's draw settings
 * @param new_xpos X coordinate to draw at
 * @param new_ypos Y coordinate to draw at
 * @param force Force a redraw?
 * @param line_heights 
 * @param calcd_line_heights 
 * @param is_last Is this it the last text_section node in the list?
 */
void handle_TEXT_STATIC(struct text_section *cur,
                        struct draw_settings *ds,
                        int16_t *new_xpos,
                        int16_t *new_ypos,
                        unsigned int *force,
                        int *line_heights,
                        int *calcd_line_heights,
                        unsigned int *is_last)
{
        if (cur->xpos != -1 && (cur->xpos == *new_xpos) && !*force)
                return;

        cur->xpos = *new_xpos;
        cur->ypos = *new_ypos;

        //printf("REDRAWING STATIC TEXT [%s]\n", cur->value);
        if (!cur->pixel_width) {
                int direction_return;
                int font_ascent_return;
                int font_descent_return;
                XCharStruct overall_return;

                XTextExtents(ds->font_struct,
                             cur->value,
                             strlen(cur->value),
                             &direction_return,
                             &font_ascent_return,
                             &font_descent_return,
                             &overall_return);

                cur->pixel_width = overall_return.width;

                /* Calculate line height. */
                if (!*calcd_line_heights) {
                        int height = overall_return.ascent +
                                     overall_return.descent;

                        if (!line_heights[cur->line])
                                line_heights[cur->line] =
                                        height + ds->min_line_spacing;

                        else if (line_heights[cur->line] < height)
                                line_heights[cur->line] =
                                        height + ds->min_line_spacing;
                }
        }

        render_queue_add(cur->value,
                         NULL,
                         ds->color,
                         ds->font,
                         cur->type,
                         0,
                         cur->xpos,
                         cur->ypos,
                         0, 0,
                         cur->xpos,
                         cur->ypos - ds->font_y_offset,
                         cur->pixel_width,
                         line_heights[cur->line],
                         is_last);
}

/** 
 * @brief Handles drawing and updating of variable text.
 * 
 * @param cur Current text_section node
 * @param ds donky's draw settings
 * @param new_xpos X coordinate to draw at
 * @param new_ypos Y coordinate to draw at
 * @param force Force a redraw?
 * @param line_heights 
 * @param calcd_line_heights 
 * @param is_last Is this it the last text_section node in the list?
 */
void handle_TEXT_VARIABLE(struct text_section *cur,
                          struct draw_settings *ds,
                          int16_t *new_xpos,
                          int16_t *new_ypos,
                          unsigned int *force,
                          int *line_heights,
                          int *calcd_line_heights,
                          unsigned int *is_last)
{
        /* if force = 0 and our x position
         * hasn't changed, respect timeouts. */
        if (!*force && (cur->xpos == *new_xpos)) {

                /* if we haven't timed out... */
                if ((get_time() - cur->last_update) < cur->timeout)
                        return;

                /* if we've updated variables with
                 * no timeout once already... */
                if (cur->timeout == 0 && (cur->last_update != 0))
                        return;
        }

        /* Crash proofing (TM)!
         *  -brought to you by lobo! (d-bag) */
        if (cur->mod_var == NULL)
                return;
        if (cur->mod_var->sym == NULL)
                return;

        cur->xpos = *new_xpos;
        cur->ypos = *new_ypos;

        switch (cur->mod_var->type) {
                case VARIABLE_STR:
                        handle_VARIABLE_STR(cur,
                                            ds,
                                            line_heights,
                                            calcd_line_heights,
                                            is_last);
                        break;
                case VARIABLE_BAR:
                        handle_VARIABLE_BAR(cur,
                                            ds,
                                            line_heights,
                                            calcd_line_heights,
                                            is_last);
                        break;
                case VARIABLE_GRAPH:
                        break;
                case VARIABLE_CUSTOM:
                        break;
                default:
                        printf("Invalid module variable_type.\n");
                        break;
        }
}

/**
 * @brief Handles TEXT_VARIABLE->VARIABLE_STR text_section types.
 *        Takes care of any text that can change from one update
 *        to the next.
 *
 * @param cur The current text_section node.
 * @param ds Current donky draw settings.
 * @param line_heights
 * @param calcd_line_heights
 * @param is_last Whether or not this is the last node
 *                in the text_section list.
 */
void handle_VARIABLE_STR(struct text_section *cur,
                         struct draw_settings *ds,
                         int *line_heights,
                         int *calcd_line_heights,
                         unsigned int *is_last)
{
        /* Get the function we need to get the variable data. */
        char *(*sym)(char *) = cur->mod_var->sym;

        int direction_return;
        int font_ascent_return;
        int font_descent_return;
        XCharStruct overall_return;

        if ((get_time() - cur->last_update >= cur->timeout) ||
            (cur->timeout == 0 && (cur->last_update == 0))) {
                memset(cur->str_result, 0, sizeof(cur->str_result));
                strncpy(cur->str_result,
                        sym(cur->args),
                        sizeof(cur->str_result) - 1);

                cur->last_update = get_time();
                //printf("Updating... %s\n", mod->name);
        }

        XTextExtents(ds->font_struct,
                     cur->str_result,
                     strlen(cur->str_result),
                     &direction_return,
                     &font_ascent_return,
                     &font_descent_return,
                     &overall_return);

        cur->pixel_width = overall_return.width;

        /* Calculate line height. */
        if (!*calcd_line_heights) {
                int height = overall_return.ascent +
                             overall_return.descent;

                if (!line_heights[cur->line])
                        line_heights[cur->line] =
                                height + ds->min_line_spacing;

                else if (line_heights[cur->line] < height)
                        line_heights[cur->line] =
                                height + ds->min_line_spacing;
        }

        render_queue_add(cur->str_result,
                         NULL,
                         ds->color,
                         ds->font,
                         cur->type,
                         cur->mod_var->type,
                         cur->xpos,
                         cur->ypos,
                         0, 0,
                         cur->xpos,
                         cur->ypos - ds->font_y_offset,
                         cur->pixel_width,
                         line_heights[cur->line],
                         is_last);
}

/**
 * @brief Handles TEXT_VARIABLE->VARIABLE_BAR text_section types.
 *        Draws bars. Duh.
 *
 * @param cur The current text_section node.
 * @param ds Current donky draw settings.
 * @param line_heights
 * @param calcd_line_heights
 * @param is_last Whether or not this is the last node
 *                in the text_section list.
 */
void handle_VARIABLE_BAR(struct text_section *cur,
                         struct draw_settings *ds,
                         int *line_heights,
                         int *calcd_line_heights,
                         unsigned int *is_last)
{
        /* Get the function we need to get the variable data. */
        int (*sym)(char *) = cur->mod_var->sym;

        int width;
        int height;
        char *rem_args = NULL;

        if ((get_time() - cur->last_update >= cur->timeout) ||
            (cur->timeout == 0 && (cur->last_update == 0))) {
                if (sscanf(cur->args, " %d %d ", &width, &height) == 2) {
                        cur->pixel_width = width;
                        cur->pixel_height = height;
                } else if (sscanf(cur->args,
                                  " %d %d %a[^\n]",
                                  &width, &height, &rem_args) == 3) {
                        cur->pixel_width = width;
                        cur->pixel_height = height;
                } else {
                        cur->pixel_width = DEFAULT_BAR_WIDTH;
                        cur->pixel_height = DEFAULT_BAR_HEIGHT;
                }

                if (rem_args) {
                        cur->int_result = sym(m_strdup(rem_args));
                        free(rem_args);
                } else {
                        cur->int_result = sym(cur->args);
                }
                        
                cur->last_update = get_time();
        }

        /* Calculate line height. */
        if (!*calcd_line_heights) {
                if (!line_heights[cur->line])
                        line_heights[cur->line] =
                                cur->pixel_height + ds->min_line_spacing;

                else if (line_heights[cur->line] < cur->pixel_height)
                        line_heights[cur->line] =
                                cur->pixel_height + ds->min_line_spacing;
        }

        /* Our y coordinate to draw at to be vertically centered. */
        int v_center = cur->ypos - ds->font_y_offset +
                       ((line_heights[cur->line] - cur->pixel_height) / 4);

        render_queue_add(NULL,
                         &cur->int_result,
                         ds->color,
                         ds->font,
                         cur->type,
                         cur->mod_var->type,
                         cur->xpos,
                         v_center,
                         cur->pixel_width - 1,
                         cur->pixel_height,
                         cur->xpos,
                         cur->ypos - ds->font_y_offset,
                         cur->pixel_width,
                         line_heights[cur->line],
                         is_last);
}

/** 
 * @brief Clean up the mess we made in this file. (Or at least try.)
 * 
 * @param x_conn donky's open x_connection
 * @param ws donky's window_settings
 * @param ds donky's draw_settings
 * @param line_heights 
 */
void clean_x(struct x_connection *x_conn,
             struct window_settings *ws,
             struct draw_settings *ds,
             int *line_heights)
{
        printf("Cleaning up... ");

        /* Cleanup. */
        freeif(line_heights);
        XFreeFontInfo(NULL, ds->font_struct, 0);
        xcb_close_font(x_conn->connection, ds->font);
        freeif(ds->font_name);
        freeif(ds->color.bg_name);
        freeif(ds->color.fg_name);
        freeif(ds);
        freeif(ws);

        /* Destroy our window and disconnect from X. */
        xcb_destroy_window(x_conn->connection, x_conn->window);
        xcb_flush(x_conn->connection);
        XCloseDisplay(x_conn->display);
        xcb_disconnect(x_conn->connection);
        freeif(x_conn);

        printf("done.\n");
}

