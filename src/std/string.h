/*	$OpenBSD: string.h,v 1.18 2007/09/03 14:36:40 millert Exp $	*/
/*	$NetBSD: string.h,v 1.6 1994/10/26 00:56:30 cgd Exp $	*/

/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)string.h	5.10 (Berkeley) 3/9/91
 */

#ifndef DSTRING_H
#define DSTRING_H

#include <string.h>

#ifndef HAVE_STRLCAT
size_t	 strlcat(char *, const char *, size_t);
#endif /* HAVE_STRLCAT */

#ifndef HAVE_STRLCPY
size_t	 strlcpy(char *, const char *, size_t);
#endif /* HAVE_STRLCPY */

#ifndef HAVE_STRCASECMP
int	 strcasecmp(const char *, const char *);
#endif /* HAVE_STRCASECMP */

#ifndef HAVE_STRNCASECMP
int	 strncasecmp(const char *, const char *, size_t);
#endif /* HAVE_STRNCASECMP */

#endif /* DSTRING_H */
