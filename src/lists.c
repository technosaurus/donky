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

#include <stdlib.h>

#include "lists.h"

/** 
 * @brief Prepares a struct of pointers used in maintaining a linked list.
 * 
 * @return Pointer to the struct.
 */
struct first_last *init_list(void)
{
        struct first_last *fl = malloc(sizeof(struct first_last));

        fl->first = NULL;
        fl->last = NULL;
        
        return fl;
}

/** 
 * @brief Adds a new node to a linked list.
 * 
 * @param data The content to be inserted into the new node.
 * @param fl The first_last struct of the list you want to add to.
 * 
 * @return Pointer to the newly created node.
 */
void *add_node(void *data, struct first_last *fl)
{
        struct list *new = malloc(sizeof(struct list));
        new->data = data;
        new->next = NULL;
        new->prev = NULL;

        if (fl->last) {
                fl->last->next = new;
                new->prev = fl->last;
                fl->last = new;
        } else {
                fl->first = fl->last = new;
        }

        return new->data;
}

/** 
 * @brief Searches for a node in the list.
 * 
 * @param match_callback Pointer to a function that checks the content of
 *                       a node to see if it matches what's held in the variable
 *                       "match". Function should return 1 if it matches,
 *                       else 0.
 * @param fallback Pointer to a function to run if a node can't be found. This
 *                 could be a function that adds and returns a new node, for
 *                 example, in effect saying overall "search for a node named
 *                 blah, and if it doesn't exist, _create_ a node named blah
 *                 and return it." This can be NULL.
 * @param match Passed to the match_callback function to be matched against.
 * @param fl The first_last struct of the list you want to search.
 * 
 * @return 
 */
void *get_node(void *match_callback,
               void *fallback,
               void *match,
               struct first_last *fl)
{
        int (*call)(void *data, void *match) = match_callback;
        void *(*fall)(void *match) = fallback;

        struct list *cur = fl->first;
        struct list *next;

        while (cur) {
                next = cur->next;

                if (call(cur->data, match))
                        return cur->data;

                cur = next;
        }

        if (fall != NULL)
                return fall(match);
        else
                return NULL;
}

/** 
 * @brief Deletes a node from a linked list.
 * 
 * @param match_callback Used in calling get_node(). See above.
 * @param free_external Pointer to a function that frees the module's content
 *                      in the node. This can be NULL if a module's custom
 *                      struct has nothing malloc'd in it. (Memory leaks = bad)
 * @param match Used in calling get_node(). See above.
 * @param fl The first_last struct of the list you want to delete from.
 */
void del_node(void *match_callback,
              void *free_external,
              void *match,
              struct first_last *fl)
{
        int (*call)(void *data, void *match) = match_callback;
        void (*free_ext)(void *data) = free_external;

        struct list *cur = get_node(call, NULL, match, fl);

        if (cur) {
                if (cur->prev)
                        cur->prev->next = cur->next;
                if (cur->next)
                        cur->next->prev = cur->prev;
                if (cur == fl->first)
                        fl->first = cur->next;
                if (cur == fl->last)
                        fl->last = cur->prev;

                if (free_ext != NULL)
                        free_ext(cur->data);

                free(cur->data);
                free(cur);
        }
}

/** 
 * @brief Deletes a linked list.
 * 
 * @param free_external See del_node() just above.
 * @param fl The first_last struct of the list you wish to delete.
 */
void del_list(void *free_external, struct first_last *fl)
{
        void (*free_ext)(void *data) = free_external;

        struct list *cur = fl->first;
        struct list *next;

        while (cur) {
                next = cur->next;

                if (free_ext != NULL)
                        free_ext(cur->data);

                free(cur->data);
                free(cur);

                cur = next;
        }

        freeif(fl);
}

/** 
 * @brief Run a function on the contents of a list.
 * 
 * @param execute Pointer to a function you wish to run on the contents
 *                of all nodes in a list.
 * @param fl The first_last struct of the list you wish to act upon.
 */
void act_on_list(void *execute, struct first_last *fl)
{
        void (*exec)(void *data) = execute;

        struct list *cur = fl->first;
        struct list *next;

        while (cur) {
                next = cur->next;

                exec(cur->data);

                cur = next;
        }
}
