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

//include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <xcb/xcb.h>
#include <render.h>

/** 
 * @brief Connects to the X server and stores all relevant information
 */
void init_X_connection(void)
{
        xcb_void_cookie_t window_cookie;
        xcb_void_cookie_t map_cookie;
        uint32_t mask;
        uint32_t values[2];
        
        /* TODO - specified colors from cfg
        char *fg_color, *bg_color; */

        /* connect to X server */
        connection = xcb_connect(NULL, &screen_number);

        if (!connection) {
                printf("Can't connect to X server.\n");
                exit(EXIT_FAILURE);
        }

        setup = xcb_get_setup(connection);

        screen = NULL;
        
        screen_iter = xcb_setup_roots_iterator(setup);

        /* find our current screen */        
        for (; screen_iter.rem != 0; screen_number--, xcb_screen_next(&screen_iter))
                if (screen_number == 0) {
                        screen = screen_iter.data;
                        break;
                }

        if (!screen) {
                printf("Can't find the current screen.\n");
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }

        /* TODO - this should only be if OwnWindow is set,
         *        otherwise draw to root(?) */
        window = xcb_generate_id(connection);
        mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
        values[0] = screen->white_pixel;
        values[1] = 
                XCB_EVENT_MASK_KEY_RELEASE |
                XCB_EVENT_MASK_BUTTON_PRESS |
                XCB_EVENT_MASK_EXPOSURE;
        window_cookie = xcb_create_window(
                        connection,
                        screen->root_depth,
                        window,
                        screen->root,
                        20, 20,   /* TODO - x/y offset from cfg */
                        1024, 50, /* TODO - width/height from cfg */
                        0,
                        XCB_WINDOW_CLASS_INPUT_OUTPUT, /* learn this */
                        screen->root_visual,
                        mask, values);
        map_cookie = xcb_map_window_checked(connection, window);

        /* some error management */
        error = xcb_request_check(connection, window_cookie);
        if (error) {
                printf("Can't create window. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }
        
        error = xcb_request_check(connection, map_cookie);
        if (error) {
                printf("Can't map window. Error: %d\n", error->error_code);
                xcb_disconnect(connection);
                exit(EXIT_FAILURE);
        }
}

/** 
 * @brief Keep the window open til we hit Escape.
 *        Just for testing purposes.
 */
void test_event(void)
{
        xcb_flush(connection);

        while (1) {
                event = xcb_poll_for_event(connection);
                if (event) {
                switch (event->response_type & ~0x80) {
                        case XCB_EXPOSE: {
                                printf("Press ESC in window to exit.\n");
                                break;
                        }
                        case XCB_KEY_RELEASE: {
                                xcb_key_release_event_t *ev;

                                ev = (xcb_key_release_event_t *)event;

                                switch (ev->detail) {
                                        /* ESC */
                                        case 9:
                                        free(event);
                                        xcb_disconnect(connection);
                                        return 0;
                                }
                        }
                }
                
                free(event);
                }
        }
}

