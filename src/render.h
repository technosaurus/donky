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

void render_text(char *str,
                 xcb_font_t font,
                 struct donky_color color,
                 int16_t x,
                 int16_t y);
void clear_area(int16_t x, int16_t y, uint16_t w, uint16_t h);
xcb_font_t get_font(char *font_name);
xcb_gc_t get_font_gc(xcb_font_t font, uint32_t pixel_bg, uint32_t pixel_fg);
void close_font(xcb_font_t font);
xcb_query_text_extents_reply_t *get_extents(char *str, xcb_font_t font);

#endif /* RENDER_H */

