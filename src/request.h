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

#ifndef REQUEST_H
#define REQUEST_H

struct request_list {
        char *id;
        const donky_conn *conn;
        struct module_var *var;
        char *args;
        bool remove;
        bool remove_now;
        bool first;
        
        struct request_list *prev;
        struct request_list *next;
};

int request_list_add(const donky_conn *conn, const char *buf, bool remove);
void request_list_remove(struct request_list *cur);
void request_list_clear(void);
int request_handler_start(void);
struct request_list *request_list_find_by_conn(donky_conn *conn);

#endif /* REQUEST_H */
