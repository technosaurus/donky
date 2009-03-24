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

#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "mem.h"
#include "util.h"

struct mem_list {
        void *ptr;
        struct mem_list *next;
};

/* Function prototypes. */
void mem_list_add(void *ptr);
void mem_list_clear(void);
void *m_malloc(size_t size);
void *m_calloc(size_t nelem, size_t size);
char *m_strdup(char *str);
char *m_strndup(char *str, size_t size);

/* Globals. */
struct mem_list *mem_start = NULL;
struct mem_list *mem_end = NULL;

/**
 * @brief Malloc wrapper.
 *
 * @param size Size of memory to allocate
 *
 * @return Pointer to memory
 */
void *m_malloc(size_t size)
{
        void *ptr = malloc(size);
        mem_list_add(ptr);
        return ptr;
}

/**
 * @brief Calloc wrapper.
 *
 * @param nelem Number of elements
 * @param size Size of each element
 *
 * @return Pointer to memory
 */
void *m_calloc(size_t nelem, size_t size)
{
        void *ptr = calloc(nelem, size);
        mem_list_add(ptr);
        return ptr;
}

/**
 * @brief Strdup wrapper.
 *
 * @param str String to duplicate
 *
 * @return Pointer to string
 */
char *m_strdup(char *str)
{
        char *ret = strdup(str);
        mem_list_add(ret);
        return ret;
}

/**
 * @brief Strndup wrapper.
 *
 * @param str String to duplicate
 * @param size Maximum size
 *
 * @return Pointer to string
 */
char *m_strndup(char *str, size_t size)
{
        char *ret = strndup(str, size);
        mem_list_add(ret);
        return ret;
}

/**
 * @brief Add pointer to linked list.
 *
 * @param ptr Pointer to add
 */
void mem_list_add(void *ptr)
{
        struct mem_list *n = malloc(sizeof(struct mem_list));

        n->ptr = ptr;
        n->next = NULL;

        /* Add to linked list. */
        if (mem_start == NULL) {
                mem_start = n;
                mem_end = n;
        } else {
                mem_end->next = n;
                mem_end = n;
        }
}

/**
 * @brief Free all nodes and data within the linked list.
 */
void mem_list_clear(void)
{
        struct mem_list *cur = mem_start, *next;

        while (cur != NULL) {
                next = cur->next;

                freeif(cur->ptr);
                free(cur);

                cur = next;
        }

        mem_start = NULL;
        mem_end = NULL;
}
