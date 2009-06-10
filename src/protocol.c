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

#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "cfg.h"
#include "daemon.h"
#include "net.h"
#include "protocol.h"
#include "request.h"
#include "util.h"

/* Function prototypes. */
static void protocol_handle_auth(donky_conn *cur, const char *buf);
static void protocol_handle_command(donky_conn *cur, const char *buf);

static void protocol_command_var(donky_conn *cur, const char *args);
static void protocol_command_varonce(donky_conn *cur, const char *args);
static void protocol_command_bye(donky_conn *cur, const char *args);
static void protocol_command_cfg(donky_conn *cur, const char *args);

/* Globals. */
donky_cmd commands[] = {
        { "var",     &protocol_command_var },
        { "varonce", &protocol_command_varonce },
        { "bye",     &protocol_command_bye },
        { "cfg",  &protocol_command_cfg },
        { NULL,      NULL }
};

/**
 * @brief This is the main protocol handler.
 *
 * @param cur Donky connection
 * @param buf Receive buffer
 */
void protocol_handle(donky_conn *cur, const char *buf)
{
        if (cur->is_authed)
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
        const char *pass = get_char_key("daemon", "pass", NULL);
        char check[64];

        /* No password needed, set user as authenticated and pass on this
         * buffer to the command handler. */
        if (pass == NULL) {
                cur->is_authed = 1;
                protocol_handle_command(cur, buf);
                return;
        }

        /* Grab the password. */
        if (sscanf(buf, PROTO_PASS_REQ, check) == 1) {
                if (!strcmp(pass, check)) {
                        cur->is_authed = 1;
                        sendcrlf(cur->sock, PROTO_PASS_ACK);
                } else {
                        sendcrlf(cur->sock, PROTO_PASS_NACK);
                }
        }
}

/**
 * @brief Handle commands.
 *
 * @param cur Donky connection
 * @param buf Receive buffer
 */
static void protocol_handle_command(donky_conn *cur, const char *buf)
{
        char *args = NULL;
        int i;
        int did = 0;    /* bool */

        args = strchr(buf, ' ');

        /* Separate args from command. */
        if (args) {
                *args = '\0';
                args++;
        }

        printf("cmd[%s]args[%s]\n", buf, args);

        /* Look for this command. */
        for (i = 0; commands[i].alias != NULL; i++) {
                if (!strcmp(commands[i].alias, buf)) {
                        did = 1;
                        commands[i].func(cur, args);
                        break;
                }
        }

        /* Send some sort of NACK if they used an unknown command. */
        if (!did)
                sendcrlf(cur->sock, PROTO_ERROR);
}

/**
 * @brief Get variable
 *
 * @param cur Donky connection
 * @param args Arguments
 */
static void protocol_command_var(donky_conn *cur, const char *args)
{
        if (args == NULL) {
                sendcrlf(cur->sock, PROTO_ERROR);
                return;
        }
        
        if ((request_list_add(cur, args, 0)))
                sendcrlf(cur->sock, PROTO_GOOD);
        else
                sendcrlf(cur->sock, PROTO_ERROR);
}

/**
 * @brief Get variable once
 *
 * @param cur Donky connection
 * @param args Arguments
 */
static void protocol_command_varonce(donky_conn *cur, const char *args)
{
        if (args == NULL) {
                sendcrlf(cur->sock, PROTO_ERROR);
                return;
        }
        
        if ((request_list_add(cur, args, 1)))
                sendcrlf(cur->sock, PROTO_GOOD);
        else
                sendcrlf(cur->sock, PROTO_ERROR);
}

/**
 * @brief Client disconnect
 *
 * @param cur Donky connection
 * @param args Arguments
 */
static void protocol_command_bye(donky_conn *cur, const char *args)
{
        sendcrlf(cur->sock, PROTO_BYE);
        donky_conn_drop(cur);
}

/**
 * @brief Config get
 *
 * @param cur Donky connection
 * @param args Arguments
 */
static void protocol_command_cfg(donky_conn *cur, const char *args)
{
        char id[8];
        char mod[64];
        char key[64];
        unsigned int type;

        if (sscanf(args, "%7[^:]:%63[^:]:%63[^:]:%u",
                   id, mod, key, &type) != 4) {
                sendcrlf(cur->sock, PROTO_ERROR);
                return;
        }

        switch (type) {
        case 0:
                sendcrlf(cur->sock, "cfg:%s:\"%s\"",
                         id, get_char_key(mod, key, ""));
                break;
        case 1:
                sendcrlf(cur->sock, "cfg:%s:%d",
                         id, get_int_key(mod, key, -1));
                break;
        case 2:
                sendcrlf(cur->sock, "cfg:%s:%f",
                         id, get_double_key(mod, key, -1.0));
                break;
        case 3:
                sendcrlf(cur->sock, "cfg:%s:%d",
                         id, get_bool_key(mod, key, -1));
                break;
        default:
                sendcrlf(cur->sock, PROTO_ERROR);
                break;
        }
}
