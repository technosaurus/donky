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

#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "../config.h"
#include "cfg.h"
#include "daemon.h"
#include "main.h"
#include "net.h"
#include "protocol.h"
#include "request.h"
#include "util.h"

/* Globals. */
static fd_set donky_fds;
static int donky_sock = -1;
static int donky_fdmax;
donky_conn *dc_start = NULL;
donky_conn *dc_end = NULL;

/* Function prototypes. */
static void donky_conn_read(donky_conn *cur);
static void donky_conn_new(donky_conn *cur);
static donky_conn *donky_conn_add(int sock);
static int donky_listen(void);
static void clean_dis_shiz(void);
static void donky_conn_set_fdmax(void);

/**
 * @brief Donky loop (tm)
 */
void donky_loop(void)
{
        fd_set fds;
        donky_conn *cur;
        
        /* Start listening, get out of here if we can't. */
        if ((donky_listen() == -1)) {
                fprintf(stderr, "I just can't listen! Ok! ;[\n");
                donky_exit = 1;
                return;
        }

        /* Zero the file descriptor sets. */
        FD_ZERO(&donky_fds);

        /* Set maximum file descriptor number. */
        donky_fdmax = donky_sock;

        /* Add the listening socket to the connection list. */
        donky_conn_add(donky_sock);

        /* Start the request handler. */
        request_handler_start();

        /* Infinite donky listener loop of death (tm) */
        while (!donky_exit && !donky_reload) {
                fds = donky_fds;

                /* Wait until we have some crap to read. */
                if ((select(donky_fdmax + 1, &fds, NULL, NULL, NULL) == -1)) {
                        perror("select");
                        break;
                }

                /* Traverse the linked list. */
                cur = dc_start;
                
                while (cur) {
                        /* Check if this connection is the one that triggered
                         * the select. */
                        if (!FD_ISSET(cur->sock, &fds)) {
                                cur = cur->next;
                                continue;
                        }
                                
                        
                        /* New connection :o */
                        if (cur->sock == donky_sock)
                                donky_conn_new(cur);
                        /* Incoming data. */
                        else
                                donky_conn_read(cur);

                        cur = cur->next;
                }
        }

        /* Run cleanup routine. */
        clean_dis_shiz();
}

/**
 * @brief Start reading from a donky connection.
 *
 * @param cur Connection to read from
 */
static void donky_conn_read(donky_conn *cur)
{
        char buf[1024];
        char *line;
        int n;
        
        n = recv(cur->sock, &buf, sizeof(buf) - 1, 0);

        if (n == -1) {
                perror("recv");
                donky_conn_drop(cur);
                return;
        } else if (n == 0) {
                DEBUGF(("Connection hung up on us!\n"));
                donky_conn_drop(cur);
                return;
        }

        buf[n] = '\0';

        /* Split up line by \r\n incase we got multiple commands at once. */
        for (line = strtok(buf, "\r\n"); line; line = strtok(NULL, "\r\n")) {
                /* Remove \r\n (if it is there) */
                chomp(line);
                chomp(line);

                /* Send away to the protocol handler. */
                DEBUGF(("line = [%s]\n", line));
                protocol_handle(cur, line);
        }
}

/**
 * @brief Handle new donky connection.
 *
 * @param cur Listener connection
 */
static void donky_conn_new(donky_conn *cur)
{
        int newfd = accept(cur->sock, NULL, NULL);

        if (newfd == -1) {
                perror("donky_client_new: accept");
                return;
        }

        if (newfd > donky_fdmax)
                donky_fdmax = newfd;

        DEBUGF(("New connection, adding to client list.\n"));
        donky_conn_add(newfd);
        sendcrlf(newfd, PROTO_CONN_ACK);
}

/**
 * @brief Add donky client to linked list.
 *
 * @param sock Socket of client
 *
 * @return Newly created node.
 */
static donky_conn *donky_conn_add(int sock)
{
        donky_conn *n = malloc(sizeof(donky_conn));

        n->sock = sock;
        n->is_authed = 0;
        
        n->prev = NULL;
        n->next = NULL;

        if (dc_end == NULL) {
                dc_start = n;
                dc_end = n;
        } else {
                dc_end->next = n;
                n->prev = dc_end;
                dc_end = n;
        }

        /* Add socket to the main fd set. */
        FD_SET(sock, &donky_fds);

        return n;
}

/**
 * @brief Drop a donky connection.
 *
 * @param cur Connection to drop
 */
void donky_conn_drop(donky_conn *cur)
{
        struct request_list *r;
        
        if (cur->prev)
                cur->prev->next = cur->next;
        if (cur->next)
                cur->next->prev = cur->prev;
        if (cur == dc_start)
                dc_start = cur->next;
        if (cur == dc_end)
                dc_end = cur->prev;

        /* This was fdmax, so lets find the next fdmax. */
        if (cur->sock == donky_fdmax)
                donky_conn_set_fdmax();

        /* Remove from fd set and close the socket. */
        FD_CLR(cur->sock, &donky_fds);
        close(cur->sock);

        /* Remove any requests this connection might have. */
        while ((r = request_list_find_by_conn(cur)))
                request_list_remove(r);

        /* Free some memorah! */
        free(cur);

        DEBUGF(("Dropped connection like a freakin' turd.\n"));
}

/**
 * @brief Find the largest sock file descriptor.
 */
static void donky_conn_set_fdmax(void)
{
        donky_conn *cur = dc_start;

        donky_fdmax = -1;

        while (cur) {
                if (cur->sock > donky_fdmax)
                        donky_fdmax = cur->sock;
                
                cur = cur->next;
        }
}

/**
 * @brief Clear the donky connections.
 */
static void donky_conn_clear(void)
{
        donky_conn *cur = dc_start;
        donky_conn *next;

        while (cur) {
                next = cur->next;

                close(cur->sock);
                free(cur);
                
                cur = next;
        }

        dc_start = NULL;
        dc_end = NULL;
}

/**
 * @brief Start listening on the configured host and port.
 */
static int donky_listen(void)
{
        const char *host = get_char_key("daemon", "host", "0.0.0.0");
        int port = get_int_key("daemon", "port", 7000);
        
        donky_sock = create_tcp_listener(host, port);

        return donky_sock;
}

/**
 * @brief Cleanup some crap in here.
 */
static void clean_dis_shiz(void)
{
        DEBUGF(("Cleaning up some daemon junk... ;[\n"));
        donky_conn_clear();
        FD_ZERO(&donky_fds);
}
