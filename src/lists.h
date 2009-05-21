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

#ifndef LISTS_H
#define LISTS_H

struct list_item {
        void *data;
        struct list_item *next;
        struct list_item *prev;
};

struct list {
        struct list_item *first;
        struct list_item *last;
};

struct list *init_list(void);
void *add_node(struct list *fl, void *data);
void *get_node(struct list *fl,
               void *match_callback,
               void *match,
               void *fallback);
void *find_node(struct list *fl, void *match_callback, void *match);
void del_node(struct list *fl,
              void *match_callback,
              void *match,
              void *free_external);
void del_nodes(struct list *fl,
               void *match_callback,
               void *match,
               void *free_external);
void del_list(struct list *fl, void *free_external);
void act_on_list(struct list *fl, void *execute);

#endif /* LISTS_H */

