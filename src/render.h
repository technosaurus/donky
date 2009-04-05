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

#ifndef RENDER_H
#define RENDER_H

#include "module.h"
#include "x11.h"

/* function prototypes */
void render_queue_exec(xcb_connection_t *connection,
                       xcb_window_t *window,
                       int16_t *window_width);
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
                      unsigned int *is_last);
uint32_t get_color(xcb_connection_t *connection,
                   xcb_screen_t *screen,
                   char *name);

#endif /* RENDER_H */

