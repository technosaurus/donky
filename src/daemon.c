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
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "config.h"
#include "daemon.h"
#include "lists.h"
#include "main.h"
#include "mem.h"
#include "protocol.h"
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
        int n;
        
        n = recv(cur->sock, &buf, sizeof(buf), 0);

        if (n == -1) {
                perror("recv");
                donky_conn_drop(cur);
                return;
        } else if (n == 0) {
                printf("Connection hung up on us!\n");
                donky_conn_drop(cur);
                return;
        }

        buf[n] = '\0';

        /* Remove \r\n (if it is there) */
        chomp(buf);
        chomp(buf);
        
        printf("buf = %s\n", buf);
        protocol_handle(cur, buf);
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

        printf("New connection, adding to client list.\n");
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
        n->authed = false;
        
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

        /* Free some memorah! */
        free(cur);

        printf("Dropped connection like a freakin' turd.\n");
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
        char *host = get_char_key("daemon", "host", NULL);
        char *port = get_char_key("daemon", "port", "7000");
        
        donky_sock = create_tcp_listener(host, port);

        freeif(host);
        freeif(port);

        return donky_sock;
}

/**
 * @brief Cleanup some crap in here.
 */
static void clean_dis_shiz(void)
{
        printf("Cleaning up some daemon junk... ;[\n");
        donky_conn_clear();
        FD_ZERO(&donky_fds);
}
