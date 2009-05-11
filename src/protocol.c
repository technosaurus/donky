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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "config.h"
#include "daemon.h"
#include "protocol.h"
#include "util.h"

/* Function prototypes. */
static void protocol_handle_auth(donky_conn *cur, const char *buf);
static void protocol_handle_command(donky_conn *cur, const char *buf);

/**
 * @brief This is the main protocol handler.
 *
 * @param cur Donky connection
 * @param buf Receive buffer
 */
void protocol_handle(donky_conn *cur, const char *buf)
{
        if (cur->authed)
                protocol_handle_command(cur, buf);
        else
                protocol_handle_auth(cur, buf);
}

/**
 * @brief Handle authentication.
 *
 * @param Donky connection
 * @param Receive buffer
 */
static void protocol_handle_auth(donky_conn *cur, const char *buf)
{
        char *pass = get_char_key("daemon", "pass", NULL);
        char *check;

        /* No password needed, set user as authenticated and pass on this
         * buffer to the command handler. */
        if (pass == NULL) {
                cur->authed = true;
                protocol_handle_command(cur, buf);
                return;
        }

        /* Grab the password. */
        if (sscanf(buf, PROTO_PASS_REQ, &check) == 1) {
                if (!strcmp(pass, check)) {
                        cur->authed = 1;
                        sendcrlf(cur->sock, PROTO_PASS_ACK);
                } else {
                        sendcrlf(cur->sock, PROTO_PASS_NACK);
                }
        }
        
        freeif(pass);
}

/**
 * @brief Handle commands.
 *
 * @param cur Donky connection
 * @param buf Receive buffer
 */
static void protocol_handle_command(donky_conn *cur, const char *buf)
{
        
}
