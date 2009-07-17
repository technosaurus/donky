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

#ifndef NET_H
#define NET_H

int sendcrlf(int sock, const char *format, ...);
int sendx(int sock, const char *format, ...);
int create_tcp_listener(const char *host, int port);

#endif /* NET_H */
