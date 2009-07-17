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

#ifndef REQUEST_H
#define REQUEST_H

#include "daemon.h"

struct request_list {
        unsigned int id;
        const donky_conn *conn;
        struct module_var *var;
        char *args;
        int remove;     /* bool */
        int is_first;   /* bool */
        char *tofree;
        
        struct request_list *prev;
        struct request_list *next;
};

int request_list_add(const donky_conn *conn, const char *buf, int remove);
void request_list_remove(struct request_list *cur);
void request_list_clear(void);
int request_handler_start(void);
struct request_list *request_list_find_by_conn(donky_conn *conn);

#endif /* REQUEST_H */
