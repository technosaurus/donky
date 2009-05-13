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

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../config.h"

#define PROTO_CONN_ACK  "donky " VERSION
#define PROTO_PASS_REQ  "pass: %m[^\r\n]"
#define PROTO_PASS_ACK  "SUP"
#define PROTO_PASS_NACK "STFU"

typedef struct {
        char *alias;
        void *func;
} donky_cmd;

void protocol_handle(donky_conn *cur, const char *buf);

#endif /* PROTOCOL_H */
