/**
 * The CC0 1.0 Universal is applied to this work.
 *
 * To the extent possible under law, Matt Hayes and Jake LeMaster have
 * waived all copyright and related or neighboring rights to donky.
 * This work is published from the United States.
 *
 * Please see the copy of the CC0 included with this program for complete
 * information including limitations and disclaimers. If no such copy
 * exists, see <http://creativecommons.org/publicdomain/zero/1.0/legalcode>.
 */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "../config.h"

#include "daemon.h"

#define PROTO_CONN_ACK  "donky " VERSION
#define PROTO_PASS_REQ  "pass: %63s[^\r\n]"
#define PROTO_PASS_ACK  "SUP"
#define PROTO_PASS_NACK "STFU"
#define PROTO_BYE       "PEACE"
#define PROTO_ERROR     "ERROR"
#define PROTO_GOOD      "GOOD"

typedef struct {
        char *alias;
        void (*func)(donky_conn *, const char *);
} donky_cmd;

void protocol_handle(donky_conn *cur, const char *buf);

#endif /* PROTOCOL_H */
