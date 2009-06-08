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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "net.h"

/**
 * @brief Send sprintf formatted string to socket, CR-LF appended.
 *
 * @param sock Socket
 * @param format Format string
 * @param ... Format arguments
 *
 * @return Bytes written to socket
 */
int sendcrlf(int sock, const char *format, ...)
{
        char buffer[2048];
        va_list ap;
        
        va_start(ap, format);
        vsnprintf(buffer, sizeof(buffer), format, ap);
        va_end(ap);

        return sendx(sock, "%s\r\n", buffer);
}

/**
 * @brief Send sprintf formatted string to socket
 *
 * @param sock Socket
 * @param format Format string
 * @param ... Format arguments
 *
 * @return Bytes written to socket
 */
int sendx(int sock, const char *format, ...)
{
        char buffer[2048];
        va_list ap;
        int n;
        
        va_start(ap, format);
        n = vsnprintf(buffer, sizeof(buffer), format, ap);
        va_end(ap);

        return send(sock, buffer, n, 0);
}

/**
 * @brief Create a TCP listening socket.
 *
 * @param host Hostname
 * @param port Port to listen on
 *
 * @return Socket!
 */
int create_tcp_listener(const char *host, int port)
{
        int sfd;
        struct sockaddr_in server;
        struct hostent *hptr;
        int opt = 1;

        if ((sfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("socket");
                return -1;
        }

        if ((hptr = gethostbyname(host)) == NULL) {
                fprintf(stderr, "Could not gethostbyname(%s)\n", host);
                return -1;
        }
        memcpy(&server.sin_addr, hptr->h_addr_list[0], hptr->h_length);

        server.sin_family = AF_INET;
        server.sin_port = htons((short) port);
        server.sin_addr.s_addr = INADDR_ANY;

        if ((bind(sfd, (struct sockaddr *) &server, sizeof(server)) == -1)) {
                perror("bind");
                close(sfd);
                return -1;
        }

        /* Allow this to be reused (needed for reloads and such) */
        if ((setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
                        &opt, sizeof(opt)) == -1)) {
                perror("setsockopt");
                close(sfd);
                return -1;
        }

        /* Start listening. */
        if ((listen(sfd, 10) == -1)) {
                perror("listen");
                close(sfd);
                return -1;
        }

        return sfd;
}

