/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../config.h"

#define PROTO_CONN_ACK  "donky " VERSION
#define PROTO_PASS_REQ  "pass: %63s[^\r\n]"
#define PROTO_PASS_ACK  "SUP"
#define PROTO_PASS_NACK "STFU"
#define PROTO_BYE       "PEACE"
#define PROTO_ERROR     "ERROR"
#define PROTO_GOOD      "GOOD"

typedef struct {
        char *alias;
        void *func;
} donky_cmd;

void protocol_handle(donky_conn *cur, const char *buf);

#endif /* PROTOCOL_H */
