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

#ifndef X11_H
#define X11_H

#include <X11/Xlib.h>

#include "text.h"

struct x_connection {
        Display *display;
        xcb_connection_t *connection;
        xcb_screen_t *screen;
        xcb_window_t window;
};

struct donky_color {
        uint32_t fg;
        uint32_t bg;
        uint32_t bg_orig;
        uint32_t fg_orig;

        char *bg_name;
        char *fg_name;
};

struct draw_settings {
        xcb_font_t font;
        XFontStruct *font_struct;
        char *font_name;

        int16_t font_x_offset;
        int16_t font_y_offset;

        int16_t minimum_line_height;
        int16_t minimum_line_spacing;

        struct donky_color color;

        struct timespec tspec;
};

struct window_settings {
        unsigned int own_window;
        unsigned int override;

        uint16_t width;
        uint16_t height;

        uint16_t x_offset;
        uint16_t y_offset;

        uint32_t bg_color;
        uint32_t fg_color;
};

/* function prototypes */
struct x_connection *init_x_connection(void);
struct window_settings *draw_window(struct x_connection *x_conn);
void donky_loop(struct x_connection *x_conn,
                struct window_settings *ws);

#endif /* X11_H */

