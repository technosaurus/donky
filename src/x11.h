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

xcb_connection_t *connection;
xcb_screen_iterator_t screen_iter;
const xcb_setup_t *setup;
xcb_screen_t *screen;
xcb_generic_error_t *error;
int screen_number;
xcb_window_t window;
xcb_generic_event_t *event;

#endif /* X11_H */

