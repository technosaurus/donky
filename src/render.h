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

struct donky_color {
        uint32_t pixel_fg;
        uint32_t pixel_bg;
};

struct render_queue {
        char *value;
        
        struct donky_color color;  /* Color of this text section. */
        xcb_font_t font;           /* Font for this text section. */
        
        int16_t *xpos;             /* Current x position. */
        int16_t *ypos;             /* Current y position. */

        int16_t cl_xpos;
        int16_t cl_ypos;
        uint16_t cl_width;
        uint16_t cl_height;
        int is_last;

        struct render_queue *next;
};

void render_queue_exec(xcb_connection_t *connection,
                       xcb_window_t *window);
void render_queue_add(char *value,
                      struct donky_color color,
                      xcb_font_t font,
                      int16_t *xpos,
                      int16_t *ypos,
                      int16_t cl_xpos,
                      int16_t cl_ypos,
                      int16_t cl_width,
                      int16_t cl_height,
                      int is_last);

#endif /* RENDER_H */

