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

#ifndef MEM_H
#define MEM_H

#include <stddef.h>

/**
 * These functions should be used in modules that wish for memory management
 * to be taken care of after each variable update.  It is highly recommended
 * that modules utilize these functions.
 */
void *m_malloc(size_t size);
void *m_calloc(size_t nelem, size_t size);
char *m_strdup(char *str);
void *m_freelater(void *ptr);

/* Free and clear the memory list. */
void mem_list_clear(void);

#endif /* MEM_H */

