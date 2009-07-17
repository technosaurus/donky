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

#ifndef DAEMON_H
#define DAEMON_H

struct donky_conn_node {
        int sock;

        int is_authed; /* bool */
        
        struct donky_conn_node *next;
        struct donky_conn_node *prev;
};

typedef struct donky_conn_node donky_conn;

void donky_loop(void);
void donky_conn_drop(donky_conn *cur);

#endif /* DAEMON_H */
