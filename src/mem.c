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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"
#include "util.h"
#include "lists.h"

/* Spewns: do not mess with this.  I'm planning on doing that context thing
 * I spoke of some time in the future.  By don't mess with this I mean, don't
 * delete this because it only has one ptr in it which could obviously just
 * be pointed to by the data ptr in the linked list crap */
struct mem_data {
        void *ptr;
};

/* internal prototypes */
static void mem_list_add(void *ptr);

/* Globals. */
static struct list *mem_fl = NULL;

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
        if (str == NULL)
                return NULL;
        
        char *ret = d_strcpy(str);
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
        if (str == NULL)
                return NULL;
        
        char *ret = d_strncpy(str, size);
        mem_list_add(ret);
        return ret;
}

/** 
 * @brief Free wrapper.
 * 
 * @param ptr Alloc'd pointer
 * 
 * @return The same pointer you sent in
 */
void *m_freelater(void *ptr)
{
        if (ptr != NULL)
                mem_list_add(ptr);

        return ptr;
}

/**
 * @brief Add pointer to linked list.
 *
 * @param ptr Pointer to add
 */
static void mem_list_add(void *ptr)
{
        struct mem_data *n = malloc(sizeof(struct mem_data));

        n->ptr = ptr;

        /* Add to linked list. */
        if (mem_fl == NULL)
                mem_fl = init_list();

        add_node(mem_fl, n);
}

/**
 * @brief Callback for the del_list thinger!
 */
static void mem_list_clear_cb(struct mem_data *cur)
{
        freeif(cur->ptr);
}

/**
 * @brief Free all nodes and data within the linked list.
 */
void mem_list_clear(void)
{
        del_list(mem_fl, &mem_list_clear_cb);
        mem_fl = NULL;
}
