/*
 * This file is part of donky.
 *
 * donky is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * donky is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with donky.  If not, see <http://www.gnu.org/licenses/>.
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

        if (!ls)
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
               void *match_callback,
               void *match,
               void *add_callback)
{
        struct list_item *cur;
        int (*m)(void *data, void *match);
        void *(*a)(void *match);

        if (!ls)
                return NULL;

        cur = ls->first;
        m = match_callback;
        a = add_callback;

        while (cur) {
                if (m && cur->data)
                        if (m(cur->data, match))
                                return cur->data;

                cur = cur->next;
        }

        if (a && match)
                return a(match);
        else
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
void *find_node(struct list *ls, void *match_callback, void *match)
{
        return get_node(ls, match_callback, match, NULL);
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
              void *match_callback,
              void *match,
              void *free_external)
{
        void *result;
        struct list_item *cur;
        void (*f)(void *data);

        result = find_node(ls, match_callback, match);
        if (!result)
                return;

        f = free_external;

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

                        freeif(cur->data);
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
               void *match_callback,
               void *match,
               void *free_external)
{
        struct list_item *cur;
        struct list_item *next;
        void *result;
        void (*f)(void *data);

        if (!ls)
                return;

        f = free_external;

        cur = ls->first;
        while (cur) {
                result = find_node(ls, match_callback, match);
                if (!result)
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

                        freeif(cur->data);
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
void del_list(struct list *ls, void *free_external)
{
        struct list_item *cur;
        struct list_item *next;
        void (*f)(void *data);

        if (!ls)
                return;
        
        f = free_external;

        cur = ls->first;
        while (cur) {
                next = cur->next;

                if (f && cur->data)
                        f(cur->data);

                freeif(cur->data);
                free(cur);

                cur = next;
        }

        freeif(ls);
}

/** 
 * @brief Run a function on the contents of a list.
 * 
 * @param ls The list struct of the list you wish to act upon.
 * @param execute_callback Pointer to a function you wish to run on the contents
 *                of all nodes in a list.
 */
void act_on_list(struct list *ls, void *execute_callback)
{
        struct list_item *cur;
        void (*e)(void *data);

        if (!ls)
                return;

        e = execute_callback;

        cur = ls->first;
        while (cur) {
                if (e)
                        e(cur->data);

                cur = cur->next;
        }
}

/**
 * @brief Run a function on the contents of a list but pass the raw 'list'
 *        structure.
 *
 * @param ls First last struct...
 * @param execute_callback Callback! Ex. void meth(struct list_item *cur);
 */
void act_on_list_raw(struct list *ls, void *execute_callback)
{
        struct list_item *cur;
        void (*e)(struct list_item *cur);

        e = execute_callback;

        cur = ls->first;
        while (cur) {
                if (e)
                        e(cur);

                cur = cur->next;
        }
}

/**
 * @brief Act on the list if the condition is met by the match_callback.
 *
 * @param ls
 * @param execute_callback
 * @param match_callback
 * @param match
 */
void act_on_list_if(struct list *ls,
                    void *execute_callback,
                    void *match_callback,
                    void *match)
{
        int (*m)(void *data, void *match);
        void (*e)(void *data);
        struct list_item *cur;

        m = match_callback;
        e = execute_callback;
        if (!m || !e)
                return;

        cur = ls->first;
        while (cur) {
                if (m(cur->data, match))
                        e(cur->data);

                cur = cur->next;
        }
}

/**
 * @brief Act on the list if the condition is met by the match_callback.
 *
 * @param ls
 * @param execute_callback
 * @param match_callback
 * @param match
 */
void act_on_list_raw_if(struct list *ls,
                        void *execute_callback,
                        void *match_callback,
                        void *match)
{
        int (*m)(struct list_item *cur, void *match);
        void (*e)(struct list_item *cur);
        struct list_item *cur;

        m = match_callback;
        e = execute_callback;
        if (!m || !e)
                return;

        cur = ls->first;
        while (cur) {
                if (m(cur, match))
                        e(cur);

                cur = cur->next;
        }
}
