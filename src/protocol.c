/*
 * Copyright (c) 2009 Matt Hayes, Jake LeMaster
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "config.h"
#include "daemon.h"
#include "protocol.h"
#include "request.h"
#include "util.h"

/* Function prototypes. */
static void protocol_handle_auth(donky_conn *cur, const char *buf);
static void protocol_handle_command(donky_conn *cur, const char *buf);

static void protocol_command_var(donky_conn *cur, const char *args);
static void protocol_command_varonce(donky_conn *cur, const char *args);
static void protocol_command_bye(donky_conn *cur, const char *args);

/* Globals. */
donky_cmd commands[] = {
        { "var",     &protocol_command_var },
        { "varonce", &protocol_command_varonce },
        { "bye",     &protocol_command_bye },
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
        char *args = NULL;
        int i;
        void *(*func)(donky_conn *, const char *);

        args = strchr(buf, ' ');

        /* Separate args from command. */
        if (args) {
                *args = '\0';
                args++;
        }

        printf("cmd[%s]args[%s]\n", buf, args);

        /* Look for this command. */
        for (i = 0; commands[i].alias != NULL; i++) {
                if (!strcasecmp(commands[i].alias, buf)) {
                        func = commands[i].func;
                        func(cur, args);
                        break;
                }
        }
}

/**
 * @brief Get variable
 *
 * @param cur Donky connection
 * @param args Arguments
 */
static void protocol_command_var(donky_conn *cur, const char *args)
{
        if ((request_list_add(cur, args, false)))
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
        if ((request_list_add(cur, args, true)))
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
