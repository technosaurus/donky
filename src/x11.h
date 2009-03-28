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

uint32_t color_bg_orig;
uint32_t color_fg_orig;

int window_width;
int window_height;

struct XFontStruct *font_struct_orig;

struct x_connection {
        Display *display;
        xcb_connection_t *connection;
        xcb_screen_t *screen;
        xcb_window_t window;
};

#endif /* X11_H */

