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
        struct list *fl = malloc(sizeof(struct list));
        fl->first = NULL;
        fl->last = NULL;
        return fl;
}

/** 
 * @brief Adds a new node to a linked list.
 * 
 * @param fl The list struct of the list you want to add to.
 * @param data The content to be inserted into the new node.
 * 
 * @return Pointer to the newly created node.
 */
void *add_node(struct list *fl, void *data)
{
        if (!fl)
                return;

        struct list_item *new = malloc(sizeof(struct list_item));
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
 * @brief Looks for a node in a list. It it isn't found, it can create and
 *        return a new one.
 * 
 * @param fl The list to search.
 * @param match_callback Pointer to a function that checks the content of a
 *                       node against whatever is the paramater "match" below.
 *                       Function should return 1 if it matches, and 0 if not.
 * @param match Passed to the match_callback function to be matched against.
 * @param otherwise Pointer to a function that will add a new node to the list
 *                  and return the new node.
 * 
 * @return The requested node.
 */
void *get_node(struct list *fl,
               void *match_callback,
               void *match,
               void *otherwise)
{
        if (!fl)
                return NULL;
        
        int (*call)(void *data, void *match) = match_callback;
        void *(*fallback)(void *match) = otherwise;

        struct list_item *cur = fl->first;

        while (cur) {
                if (call && cur->data)
                        if (call(cur->data, match))
                                return cur->data;

                cur = cur->next;
        }

        if (fallback && match)
                return fallback(match);
        else
                return NULL;
}

/** 
 * @brief Convenient and straight "find a node and return NULL if it doesn't
 *        exist" function.
 * 
 * @param fl See above.
 * @param match_callback See above.
 * @param match  See above.
 * 
 * @return The requested node if it exists, else NULL.
 */
void *find_node(struct list *fl, void *match_callback, void *match)
{
        return get_node(fl, match_callback, match, NULL);
}

/** 
 * @brief Deletes a node from a linked list.
 * 
 * @param fl The list struct of the list you want to delete from.
 * @param match_callback Used in calling get_node(). See above.
 * @param match Used in calling get_node(). See above.
 * @param free_external Pointer to a function that frees the module's content
 *                      in the node. This can be NULL if a module's custom
 *                      struct has nothing malloc'd in it. (Memory leaks = bad)
 */
void del_node(struct list *fl,
              void *match_callback,
              void *match,
              void *free_external)
{
        if (!fl)
                return;
        
        void (*free_ext)(void *data) = free_external;

        void *result = find_node(fl, match_callback, match);
        struct list_item *cur = fl->first;

        if (!result)
                return;
                
        while (cur) {
                if (cur->data == result) {
                        if (cur->prev)
                                cur->prev->next = cur->next;
                        if (cur->next)
                                cur->next->prev = cur->prev;
                        if (cur == fl->first)
                                fl->first = cur->next;
                        if (cur == fl->last)
                                fl->last = cur->prev;

                        if (free_ext && cur->data)
                                free_ext(cur->data);

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
 * @param fl
 * @param match_callback
 * @param match
 * @param free_external
 */
void del_nodes(struct list *fl,
               void *match_callback,
               void *match,
               void *free_external)
{
        if (!fl)
                return;
        
        void (*free_ext)(void *data) = free_external;

        void *result;
        struct list_item *cur = fl->first;
        struct list_item *next;
                
        while (cur) {
                result = find_node(fl, match_callback, match);
                if (!result)
                        return;
                
                next = cur->next;
                
                if (cur->data == result) {
                        if (cur->prev)
                                cur->prev->next = cur->next;
                        if (cur->next)
                                cur->next->prev = cur->prev;
                        if (cur == fl->first)
                                fl->first = cur->next;
                        if (cur == fl->last)
                                fl->last = cur->prev;

                        if (free_ext && cur->data)
                                free_ext(cur->data);

                        freeif(cur->data);
                        free(cur);
                }

                cur = next;
        }
}

/** 
 * @brief Deletes a linked list.
 * 
 * @param fl The list struct of the list you wish to delete.
 * @param free_external See del_node() just above.
 */
void del_list(struct list *fl, void *free_external)
{
        if (!fl)
                return;
        
        void (*free_ext)(void *data) = free_external;

        struct list_item *cur = fl->first;
        struct list_item *next;

        while (cur) {
                next = cur->next;

                if (free_ext && cur->data)
                        free_ext(cur->data);

                freeif(cur->data);
                free(cur);

                cur = next;
        }

        freeif(fl);
}

/** 
 * @brief Run a function on the contents of a list.
 * 
 * @param fl The list struct of the list you wish to act upon.
 * @param execute Pointer to a function you wish to run on the contents
 *                of all nodes in a list.
 */
void act_on_list(struct list *fl, void *execute)
{
        void (*exec)(void *data) = execute;

        struct list_item *cur = fl->first;

        while (cur) {
                if (exec)
                        exec(cur->data);

                cur = cur->next;
        }
}

/**
 * @brief Run a function on the contents of a list but pass the raw 'list'
 *        structure.
 *
 * @param fl First last struct...
 * @param execute Callback! Ex. void meth(struct list_item *cur);
 */
void act_on_list_raw(struct list *fl, void *execute)
{
        void (*exec)(struct list_item *cur) = execute;

        struct list_item *cur = fl->first;

        while (cur) {
                if (exec)
                        exec(cur);

                cur = cur->next;
        }
}

/**
 * @brief Act on the list if the condition is met by the match_callback.
 *
 * @param fl first last thing
 * @param execute
 * @param match_callback
 * @param match
 */
void act_on_list_if(struct list *fl, void *execute,
                    void *match_callback, void *match)
{
        int (*call)(void *data, void *match) = match_callback;
        void (*exec)(void *data) = execute;

        struct list_item *cur = fl->first;

        while (cur) {
                if (call && call(cur->data, match))
                        if (exec)
                                exec(cur->data);

                cur = cur->next;
        }
}

/**
 * @brief Act on the list if the condition is met by the match_callback.
 *
 * @param fl
 * @param execute
 * @param match_callback
 * @param match
 */
void act_on_list_raw_if(struct list *fl, void *execute,
                        void *match_callback, void *match)
{
        int (*call)(struct list_item *cur, void *match) = match_callback;
        void (*exec)(struct list_item *cur) = execute;

        struct list_item *cur = fl->first;

        while (cur) {
                if (call && call(cur, match))
                        if (exec)
                                exec(cur);

                cur = cur->next;
        }
}
