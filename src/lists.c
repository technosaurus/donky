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

#include <stdio.h>
#include <stdlib.h>

#include "lists.h"
#include "util.h"

/** 
 * @brief Prepares a struct of pointers used in maintaining a linked list.
 * 
 * @return Pointer to the struct.
 */
struct list *init_list(void)
{
        struct list *ls;

        ls = malloc(sizeof(struct list));
        ls->first = NULL;
        ls->last = NULL;

        return ls;
}

/** 
 * @brief Adds a new node to a linked list.
 * 
 * @param ls The list struct of the list you want to add to.
 * @param data The content to be inserted into the new node.
 * 
 * @return Pointer to the newly created node.
 */
void *add_node(struct list *ls, void *data)
{
        struct list_item *new;

        if (ls == NULL)
                return NULL;

        new = malloc(sizeof(struct list_item));
        new->data = data;
        new->next = NULL;
        new->prev = NULL;

        if (ls->last) {
                ls->last->next = new;
                new->prev = ls->last;
                ls->last = new;
        } else {
                ls->first = ls->last = new;
        }

        return new->data;
}

/** 
 * @brief Looks for a node in a list. It it isn't found, it can create and
 *        return a new one.
 * 
 * @param ls The list to search.
 * @param match_callback Pointer to a function that checks the content of a
 *                       node against whatever is the paramater "match" below.
 *                       Function should return 1 if it matches, and 0 if not.
 * @param match Passed to the match_callback function to be matched against.
 * @param otherwise Pointer to a function that will add a new node to the list
 *                  and return the new node.
 * 
 * @return The requested node.
 */
void *get_node(struct list *ls,
               int (*m)(void *data, void *match),
               void *match,
               void *(*a)(void *match))
{
        struct list_item *cur;

        if ((ls == NULL) || (match == NULL) || (m == NULL))
                return NULL;

        cur = ls->first;
        while (cur) {
                if (cur->data && (m(cur->data, match)))
                        return cur->data;
                cur = cur->next;
        }

        if (a != NULL)
                return a(match);

        return NULL;
}

/** 
 * @brief Convenient and straight "find a node and return NULL if it doesn't
 *        exist" function.
 * 
 * @param ls See above.
 * @param match_callback See above.
 * @param match  See above.
 * 
 * @return The requested node if it exists, else NULL.
 */
void *find_node(struct list *ls,
                int (*m)(void *data, void *match),
                void *match)
{
        return get_node(ls, m, match, NULL);
}

/** 
 * @brief Deletes a node from a linked list.
 * 
 * @param ls The list struct of the list you want to delete from.
 * @param match_callback Used in calling get_node(). See above.
 * @param match Used in calling get_node(). See above.
 * @param free_external Pointer to a function that frees the module's content
 *                      in the node. This can be NULL if a module's custom
 *                      struct has nothing malloc'd in it. (Memory leaks = bad)
 */
void del_node(struct list *ls,
              int (*m)(void *data, void *match),
              void *match,
              void (*f)(void *data))
{
        void *result;
        struct list_item *cur;

        result = get_node(ls, m, match, NULL);
        if (result == NULL)
                return;

        cur = ls->first;
        while (cur) {
                if (cur->data == result) {
                        if (cur->prev)
                                cur->prev->next = cur->next;
                        if (cur->next)
                                cur->next->prev = cur->prev;
                        if (cur == ls->first)
                                ls->first = cur->next;
                        if (cur == ls->last)
                                ls->last = cur->prev;

                        if (f && cur->data)
                                f(cur->data);

                        free(cur->data);
                        free(cur);

                        return;
                }

                cur = cur->next;
        }
}

/**
 * @brief Delete all nodes matching.
 *
 * @param ls
 * @param match_callback
 * @param match
 * @param free_external
 */
void del_nodes(struct list *ls,
               int (*m)(void *data, void *match),
               void *match,
               void (*f)(void *data))
{
        struct list_item *cur;
        struct list_item *next;
        void *result;

        if (ls == NULL)
                return;

        cur = ls->first;
        while (cur) {
                result = get_node(ls, m, match, NULL);
                if (result == NULL)
                        return;

                next = cur->next;

                if (cur->data == result) {
                        if (cur->prev)
                                cur->prev->next = cur->next;
                        if (cur->next)
                                cur->next->prev = cur->prev;
                        if (cur == ls->first)
                                ls->first = cur->next;
                        if (cur == ls->last)
                                ls->last = cur->prev;

                        if (f && cur->data)
                                f(cur->data);

                        free(cur->data);
                        free(cur);
                }

                cur = next;
        }
}

/** 
 * @brief Deletes a linked list.
 * 
 * @param ls The list struct of the list you wish to delete.
 * @param free_external See del_node() just above.
 */
void del_list(struct list *ls, void (*f)(void *data))
{
        struct list_item *cur;
        struct list_item *next;

        if (ls == NULL)
                return;

        cur = ls->first;
        while (cur) {
                next = cur->next;

                if (f && cur->data)
                        f(cur->data);

                free(cur->data);
                free(cur);

                cur = next;
        }

        free(ls);
}

/** 
 * @brief Run a function on the contents of a list.
 * 
 * @param ls The list struct of the list you wish to act upon.
 * @param execute_callback Pointer to a function you wish to run on the contents
 *                of all nodes in a list.
 */
void act_on_list(struct list *ls, void (*e)(void *data))
{
        struct list_item *cur;

        if ((ls == NULL) || (e == NULL))
                return;

        cur = ls->first;
        while (cur) {
                e(cur->data);
                cur = cur->next;
        }
}
