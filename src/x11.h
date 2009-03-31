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
#include "render.h"

int window_width;
int window_height;

struct x_connection {
        Display *display;
        xcb_connection_t *connection;
        xcb_screen_t *screen;
        xcb_window_t window;
};

struct donky_draw_settings {
        xcb_font_t font;
        XFontStruct *font_struct;
        char *font_name;

        struct donky_color color;
        uint32_t color_bg_orig;
        uint32_t color_fg_orig;

        char *color_name_bg;
        char *color_name_fg;

        int16_t font_x_offset;
        int16_t font_y_offset;

        struct timespec tspec;
};

#endif /* X11_H */

