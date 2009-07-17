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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mem.h"

struct mem_data {
        void *ptr;
        struct mem_data *next;
};

/* internal prototypes */
static void mem_list_add(void *ptr);

/* Globals. */
static struct mem_data *mem_start = NULL;
static struct mem_data *mem_end = NULL;

/**
 * @brief Malloc wrapper.
 *
 * @param size Size of memory to allocate
 *
 * @return Pointer to memory
 */
void *m_malloc(size_t size)
{
        void *ptr;

        ptr = malloc(size);
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
        void *ptr;

        ptr = calloc(nelem, size);
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

        ret = strdup(str);
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
        struct mem_data *n;

        n = malloc(sizeof(struct mem_data));

        n->ptr = ptr;
        n->next = NULL;

        /* Add to linked list. */
        if (mem_end == NULL) {
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
        struct mem_data *cur;
        struct mem_data *next;

        cur = mem_start;

        while (cur != NULL) {
                next = cur->next;
                free(cur->ptr);
                free(cur);
                cur = next;
        }

        mem_start = NULL;
        mem_end = NULL;
}
