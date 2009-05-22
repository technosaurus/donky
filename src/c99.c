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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#include "c99.h"

#ifndef HAVE_STRCASECMP
/**
 * @brief Compare two strings case insensitively.  Note: I return -1 if not
 *        equal rather than the greater than less than crap. 0 means equal.
 *
 * @param s1 string 1
 * @param s2 string 2
 *
 * @return -1 for no match, 0 for match
 */
int strcasecmp(const char *s1, const char *s2)
{
        for (; (*s1 || *s2); s1++, s2++)
                if (tolower(*s1) != tolower(*s2))
                        return -1;

        return 0;
}
#endif /* HAVE_STRCASECMP */
