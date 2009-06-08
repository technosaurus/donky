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
static struct list *mem_ls = NULL;

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
        char *ret;

        if (str == NULL)
                return NULL;

        ret = dstrdup(str);
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
        if (mem_ls == NULL)
                mem_ls = init_list();

        add_node(mem_ls, n);
}

/**
 * @brief Callback for the del_list thinger!
 */
static void mem_list_clear_cb(struct mem_data *cur)
{
        freenull(cur->ptr);
}

/**
 * @brief Free all nodes and data within the linked list.
 */
void mem_list_clear(void)
{
        del_list(mem_ls, &mem_list_clear_cb);
        mem_ls = NULL;
}
