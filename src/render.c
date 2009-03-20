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

#include "X11.h"
#include "render.h"
#include "config.h"

int32_t render_text(const char *str, const char *font_name, int16_t x, int16_t y);
static xcb_font_t get_font(const char *font_name);
static xcb_gc_t get_font_gc(xcb_font_t font);

int32_t render_text(const char *str, const char *font_name, int16_t x, int16_t y)
{
        xcb_void_cookie_t cookie_gc;
        xcb_void_cookie_t cookie_text;
        xcb_font_t font;

        xcb_query_text_extents_cookie_t cookie_extents;
        xcb_query_text_extents_reply_t *extents_reply;
        int32_t extents_length = 1;

        xcb_gcontext_t gc;
        
        uint8_t length;
        length = strlen(str);

        font = get_font(font_name);

/*
        cookie_extents = xcb_query_text_extents(connection,
                                                font,
                                                strlen(str),
                                                str);

        extents_reply = xcb_query_text_extents_reply(connection,
                                                     cookie_extents,
                                                     error);

        if (error) {
                printf("render_text: Can't get extents reply. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }

        extents_length = extents_reply->overall_width;
*/

        gc = get_font_gc(font);

        cookie_text = xcb_image_text_8_checked(connection,
                                               length,
                                               window,
                                               gc,
                                               x, y,
                                               str);

        error = xcb_request_check(connection, cookie_text);
        if (error) {
                printf("render_text: Can't paste text. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }

        cookie_gc = xcb_free_gc(connection, gc);

        error = xcb_request_check(connection, cookie_gc);
        if (error) {
                printf("render_text: Can't free gc. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }

        return extents_length;

}

static xcb_font_t get_font(const char *font_name)
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
                printf("get_font: Can't open font. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }
        xcb_flush(connection);
        return font;
}

static xcb_gc_t get_font_gc(xcb_font_t font)
{
        xcb_gcontext_t gc;
        xcb_void_cookie_t cookie_gc;
        
        xcb_void_cookie_t cookie_font;

        uint32_t mask;
        uint32_t value_list[3];

        gc = xcb_generate_id(connection);
        
        mask = XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
        value_list[0] = fg_color;
        value_list[1] = bg_color;
        value_list[2] = font;

        cookie_gc = xcb_create_gc_checked(connection,
                                          gc,
                                          window,
                                          mask, value_list);
        error = xcb_request_check(connection, cookie_gc);
        if (error) {
                printf("get_font: Can't create graphic context. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }

        cookie_font = xcb_close_font_checked(connection, font);
        error = xcb_request_check(connection, cookie_font);
        if (error) {
                printf("get_font: Can't close font. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }

        return gc;
}

