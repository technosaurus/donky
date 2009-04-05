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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>

#include "render.h"
#include "config.h"
#include "util.h"

/* Function prototypes. */
void render_text(xcb_connection_t *connection,
                 xcb_window_t *window,
                 char *str,
                 xcb_font_t font,
                 struct donky_color color,
                 int16_t x,
                 int16_t y);
void render_bar(xcb_connection_t *connection,
                xcb_window_t *window,
                int *value,
                xcb_font_t font,
                struct donky_color color,
                int16_t x,
                int16_t y,
                uint16_t w,
                uint16_t h);
xcb_gc_t get_font_gc(xcb_connection_t *connection,
                     xcb_window_t *window,
                     xcb_font_t font,
                     uint32_t bg, uint32_t fg);

/* Globals. */
struct render_queue *rq_start = NULL;
struct render_queue *rq_end = NULL;

/**
 * @brief Render everything in the render queue.
 */
void render_queue_exec(xcb_connection_t *connection,
                       xcb_window_t *window,
                       int16_t *window_width)
{
        struct render_queue *cur = rq_start, *next;

        while (cur != NULL) {
                next = cur->next;

                if (cur->is_last && cur->cl_width > 0 && cur->cl_height > 0)
                    xcb_clear_area(connection,
                                   0,
                                   *window,
                                   cur->cl_xpos,
                                   cur->cl_ypos,
                                   *window_width - cur->cl_xpos,
                                   cur->cl_height);
                else if (cur->cl_width > 0 && cur->cl_height > 0)
                    xcb_clear_area(connection,
                                   0,
                                   *window,
                                   cur->cl_xpos,
                                   cur->cl_ypos,
                                   cur->cl_width,
                                   cur->cl_height);

                switch (cur->ts_type) {
                case TEXT_STATIC:
                        render_text(connection,
                                    window,
                                    cur->value,
                                    cur->font,
                                    cur->color,
                                    cur->xpos,
                                    cur->ypos);
                        break;
                case TEXT_VARIABLE:
                        switch (cur->v_type) {
                        case VARIABLE_STR:
                                render_text(connection,
                                            window,
                                            cur->value,
                                            cur->font,
                                            cur->color,
                                            cur->xpos,
                                            cur->ypos);
                                break;
                        case VARIABLE_BAR:
                                render_bar(connection,
                                           window,
                                           cur->int_value,
                                           cur->font,
                                           cur->color,
                                           cur->xpos,
                                           cur->ypos,
                                           cur->width,
                                           cur->height);
                                break;
                        case VARIABLE_GRAPH:
                                break;
                        }
                        break;
                }
                

                free(cur);
                
                cur = next;
        }

        rq_start = NULL;
        rq_end = NULL;
}

/**
 * @brief Add item to render queue.
 *
 * @param value Value of string
 * @param font Font to use
 * @param xpos X position
 * @param ypos Y position
 */
void render_queue_add(char *value,
                      int *int_value,
                      struct donky_color color,
                      xcb_font_t font,
                      enum text_section_type ts_type,
                      enum variable_type v_type,
                      int16_t xpos,
                      int16_t ypos,
                      uint16_t width,
                      uint16_t height,
                      int16_t cl_xpos,
                      int16_t cl_ypos,
                      uint16_t cl_width,
                      uint16_t cl_height,
                      unsigned int *is_last)
{
        struct render_queue *n = malloc(sizeof(struct render_queue));

        n->value = value;
        n->int_value = int_value;
        n->color = color;
        n->font = font;
        n->ts_type = ts_type;
        n->v_type = v_type;
        
        n->xpos = xpos;
        n->ypos = ypos;
        n->width = width;
        n->height = height;

        n->cl_xpos = cl_xpos;
        n->cl_ypos = cl_ypos;
        n->cl_width = cl_width;
        n->cl_height = cl_height;
        n->is_last = *is_last;
        
        n->next = NULL;
        
        /* Add to linked list. */
        if (rq_start == NULL) {
                rq_start = n;
                rq_end = n;
        } else {
                rq_end->next = n;
                rq_end = n;
        }
}

/** 
 * @brief Print a string to donky
 * 
 * @param str String to render
 * @param font Opened font to use
 * @param x where to render string on X axis
 * @param y where to render string on Y axis
 * 
 * @return Pixel length of printed string
 */
void render_text(xcb_connection_t *connection,
                 xcb_window_t *window,
                 char *str,
                 xcb_font_t font,
                 struct donky_color color,
                 int16_t x,
                 int16_t y)
{
        xcb_gcontext_t gc;
        
        uint8_t length;
        length = strlen(str);

        gc = get_font_gc(connection,
                         window,
                         font,
                         color.bg, color.fg);

        xcb_image_text_8(connection,
                         length,
                         *window,
                         gc,
                         x, y,
                         str);

        xcb_free_gc(connection, gc);
}

/**
 * @brief Render a bar in the donky window.
 *
 * @param connection XCB connection object
 * @param window XCB window
 * @param value Integer value, representing percent
 * @param font XCB font, is only needed so i can use the get_font_gc func.
 * @param color Color structure
 * @param x X position
 * @param y Y position
 * @param w Width
 * @param h Height
 */
void render_bar(xcb_connection_t *connection,
                xcb_window_t *window,
                int *value,
                xcb_font_t font,
                struct donky_color color,
                int16_t x,
                int16_t y,
                uint16_t w,
                uint16_t h)
{
        xcb_gcontext_t gc = get_font_gc(connection,
                                        window,
                                        font,
                                        color.bg, color.fg);

        int fill_width = (int)((double)w * ((double) *value / (double) 100));

        xcb_rectangle_t rect_outline[] = { x, y, w, h };
        xcb_rectangle_t rect_fill[] = { x, y, fill_width, h };

        xcb_poly_rectangle(connection, *window, gc, 1, rect_outline);
        xcb_poly_fill_rectangle(connection, *window, gc, 1, rect_fill);

        xcb_free_gc(connection, gc);
}

/**
 * @brief Allocate and return a friggin' color.
 *
 * @param name Color name
 *
 * @return Color's pixel.
 */
uint32_t get_color(xcb_connection_t *connection,
                   xcb_screen_t *screen,
                   char *name)
{
        xcb_generic_error_t *error;

        xcb_alloc_named_color_reply_t *reply;
        reply = xcb_alloc_named_color_reply(connection,
                                            xcb_alloc_named_color(connection,
                                            screen->default_colormap,
                                            strlen(name),
                                            name),
                                            &error);
        
        if (error) {
                printf("get_color: Can't allocate color. Error: %d\n",
                       error->error_code);
                freeif(reply);
                exit(EXIT_FAILURE);
        }

        uint32_t my_pixel = reply->pixel;
        freeif(reply);

        return my_pixel;
}

/** 
 * @brief Create a font graphic context
 * 
 * @param font Font to use in creation
 * 
 * @return Font graphic context
 */
xcb_gc_t get_font_gc(xcb_connection_t *connection,
                     xcb_window_t *window,
                     xcb_font_t font,
                     uint32_t bg, uint32_t fg)
{
        xcb_generic_error_t *error;
        xcb_gcontext_t gc;
        xcb_void_cookie_t cookie_gc;

        uint32_t mask;
        uint32_t value_list[3];

        gc = xcb_generate_id(connection);
        
        mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
        value_list[0] = fg;
        value_list[1] = bg;
        value_list[2] = font;

        cookie_gc = xcb_create_gc_checked(connection,
                                          gc,
                                          *window,
                                          mask, value_list);
        error = xcb_request_check(connection, cookie_gc);
        if (error)
                printf("get_font: Can't create graphic context. Error: %d\n", error->error_code);

        return gc;
}

