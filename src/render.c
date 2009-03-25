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

#include "x11.h"
#include "render.h"
#include "config.h"
#include "util.h"

/* Function prototypes. */
void render_text(char *str,
                 xcb_font_t font,
                 struct donky_color color,
                 int16_t x,
                 int16_t y);
void clear_area(int16_t x, int16_t y, uint16_t w, uint16_t h);
xcb_font_t get_font(char *font_name);
xcb_gc_t get_font_gc(xcb_font_t font, uint32_t pixel_bg, uint32_t pixel_fg);
xcb_char2b_t *build_chars(char *str, uint8_t length);
void render_queue_exec(void);
void render_queue_add(char *value,
                      struct donky_color color,
                      xcb_font_t font,
                      int16_t *xpos,
                      int16_t *ypos);
void clear_queue_exec(void);
void clear_queue_add(int16_t xpos,
                     int16_t ypos,
                     uint16_t width,
                     uint16_t height);

/* Globals. */
struct render_queue *rq_start = NULL;
struct render_queue *rq_end = NULL;

struct clear_queue *cq_start = NULL;
struct clear_queue *cq_end = NULL;

/**
 * @brief Render everything in the render queue.
 */
void clear_queue_exec(void)
{
        struct clear_queue *cur = cq_start, *next;

        while (cur != NULL) {
                next = cur->next;

                clear_area(cur->xpos,
                           cur->ypos,
                           cur->width,
                           cur->height);

                free(cur);
                
                cur = next;
        }

        cq_start = NULL;
        cq_end = NULL;
}

/**
 * @brief Add item to clear queue.
 *
 * @param xpos X position
 * @param ypos Y position
 * @param width Width
 * @param height Height
 */
void clear_queue_add(int16_t xpos,
                     int16_t ypos,
                     uint16_t width,
                     uint16_t height)
{
        struct clear_queue *n = malloc(sizeof(struct clear_queue));

        n->xpos = xpos;
        n->ypos = ypos;
        n->width = width;
        n->height = height;
        n->next = NULL;
        
        /* Add to linked list. */
        if (cq_start == NULL) {
                cq_start = n;
                cq_end = n;
        } else {
                cq_end->next = n;
                cq_end = n;
        }
}

/**
 * @brief Render everything in the render queue.
 */
void render_queue_exec(void)
{
        struct render_queue *cur = rq_start, *next;

        while (cur != NULL) {
                next = cur->next;

                render_text(cur->value,
                            cur->font,
                            cur->color,
                            *cur->xpos,
                            *cur->ypos);

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
                      struct donky_color color,
                      xcb_font_t font,
                      int16_t *xpos,
                      int16_t *ypos)
{
        struct render_queue *n = malloc(sizeof(struct render_queue));

        n->value = value;
        n->color = color;
        n->font = font;
        n->xpos = xpos;
        n->ypos = ypos;
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
void render_text(char *str,
                 xcb_font_t font,
                 struct donky_color color,
                 int16_t x,
                 int16_t y)
{
        xcb_gcontext_t gc;
        
        uint8_t length;
        length = strlen(str);

        gc = get_font_gc(font, color.pixel_bg, color.pixel_fg);

        xcb_image_text_8(connection,
                         length,
                         window,
                         gc,
                         x, y,
                         str);

        xcb_free_gc(connection, gc);
}

/**
 * @brief Clear an area of the window.
 *
 * @param x X pos
 * @param y Y pos
 * @param w Width
 * @param h Height
 */
void clear_area(int16_t x, int16_t y, uint16_t w, uint16_t h)
{
        xcb_clear_area(connection,
                       0,
                       window,
                       x, y, w, h);
}

/**
 * @brief Allocate and return a friggin' color.
 *
 * @param name Color name
 *
 * @return Color's pixel.
 */
uint32_t get_color(char *name)
{
        xcb_alloc_named_color_reply_t *reply = xcb_alloc_named_color_reply(
                connection,
                xcb_alloc_named_color(connection,
                                      screen->default_colormap,
                                      strlen(name),
                                      name),
                &error
        );
        
        if (error) {
                printf("get_color: Can't allocate color. Error: %d\n",
                       error->error_code);
                freeif(reply);
                return color_fg_orig;
        }

        uint32_t my_pixel = reply->pixel;
        freeif(reply);

        return my_pixel;
}

/** 
 * @brief Open and return a font
 * 
 * @param font_name Name of font to open
 * 
 * @return Opened font
 */
xcb_font_t get_font(char *font_name)
{
        xcb_font_t font;
        xcb_void_cookie_t cookie_font;

        font = xcb_generate_id(connection);
        cookie_font = xcb_open_font_checked(connection,
                                            font,
                                            strlen(font_name),
                                            font_name);

        error = xcb_request_check(connection, cookie_font);
        if (error) {
                printf("get_font: Can't open font [%s]. Error: %d\n",
                       font_name,
                       error->error_code);
                return font_orig;
        }

        return font;
}

/** 
 * @brief Create a font graphic context
 * 
 * @param font Font to use in creation
 * 
 * @return Font graphic context
 */
xcb_gc_t get_font_gc(xcb_font_t font, uint32_t pixel_bg, uint32_t pixel_fg)
{
        xcb_gcontext_t gc;
        xcb_void_cookie_t cookie_gc;

        uint32_t mask;
        uint32_t value_list[3];

        uint32_t font_bgcolor;
        uint32_t font_fgcolor;

        font_bgcolor = pixel_bg;
        font_fgcolor = pixel_fg;

        gc = xcb_generate_id(connection);
        
        mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
        value_list[0] = font_fgcolor;
        value_list[1] = font_bgcolor;
        value_list[2] = font;

        cookie_gc = xcb_create_gc_checked(connection,
                                          gc,
                                          window,
                                          mask, value_list);
        error = xcb_request_check(connection, cookie_gc);
        if (error)
                printf("get_font: Can't create graphic context. Error: %d\n", error->error_code);

        return gc;
}

/** 
 * @brief Closes an open font
 * 
 * @param font Font to close
 */
void close_font(xcb_font_t font)
{
        xcb_close_font(connection, font);
}

/** 
 * @brief Uh... what lobo said...
 * 
 * @param str String to convert
 * @param length Length of string
 * 
 * @return Insanity
 */
xcb_char2b_t *build_chars(char *str, uint8_t length)
{
        xcb_char2b_t *ret = malloc(length * sizeof(xcb_char2b_t));
        int i;
        
        for (i = 0; i < length; i++) {
                ret[0].byte1 = 0;
                ret[0].byte2 = str[i];
        }

        return ret;
}

/**
 * @brief Get the text extents of a given string and font.
 *
 * @param str String...
 * @param font Font structure
 *
 * @return The text extents reply structure.
 */
xcb_query_text_extents_reply_t *get_extents(char *str, xcb_font_t font)
{
        uint8_t length;
        length = strlen(str);

        xcb_char2b_t *chars;
        xcb_query_text_extents_cookie_t cookie_extents;
        xcb_query_text_extents_reply_t *extents_reply;

        chars = build_chars(str, length);

        cookie_extents = xcb_query_text_extents(connection,
                                                font,
                                                strlen(str),
                                                chars);

        extents_reply = xcb_query_text_extents_reply(connection,
                                                     cookie_extents,
                                                     &error);

        if (error)
                printf("render_text: Can't get extents reply. Error: %d\n", error->error_code);

        freeif(chars);

        return extents_reply;
}
