/*
 * $Id: salloc.c,v 1.1 2003/10/03 14:48:57 awgn Exp $
 *
 * Copyright (c) 2003 Bonelli Nicola <bonelli@antifork.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer. 2.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 */

#include <sys/queue.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sysexits.h>

#include "salloc.h"

static LIST_HEAD(listhead, entry) head;

	struct entry {
		void *callid;
		void *space;
		       LIST_ENTRY(entry) entries;	/* List. */
	};


static
struct entry *search_callid(void *addr)
{
	struct entry *np;
	for (np = head.lh_first; np != NULL; np = np->entries.le_next)
		if (np->callid == addr)
			break;
	return np;
}


__inline
void *
__alloc_stpc(size_t size)
{
	static int stpc_init;
	struct entry *p;

	if (stpc_init++ == 0)
		LIST_INIT(&head);

	if ((p = search_callid(__builtin_return_address(1))) == NULL) {
		if ((p = malloc(sizeof(struct entry)))==NULL)
			errx(EX_SOFTWARE, "%s",__FUNCTION__);

		p->callid = __builtin_return_address(1);
		p->space = malloc(size);
		LIST_INSERT_HEAD(&head, p, entries);
		return p->space;
	}

	/*
	 * realloc the space, the contents will be unchanged to the minimum
	 * of the old and new sizes
	 */
	p->space = realloc(p->space, size);

	return p->space;
}


__inline
void *
__alloc_norm(size_t size)
{
	return malloc(size);
}


__inline
void *
__alloc_stic(size_t size)
{
	static void *ptr;
	return realloc(ptr, size);
}


void *
salloc(size_t size, int mode)
{
	switch (mode) {
	case SA_NORM:
		return __alloc_norm(size);
	case SA_STIC:
		return __alloc_stic(size);
	case SA_STPC:
		return __alloc_stpc(size);
	default:
		errx(EX_SOFTWARE, "%s(): Mode error", __FUNCTION__);
	}
	/* unreachable */
	return (void *) 0;
}

#ifdef TEST_SALLOC

int 
main(argc, argv)
	int argc;
	char **argv;
{
	int i;
	char *a, *b;

	printf("%s:TEST_SALLOC\n",__FILE__);

	printf("SA_NORM:\n");

	a = salloc(64,SA_NORM);
	printf("   a@%p\n", a);

        a = salloc(64,SA_NORM);
        printf("   a@%p\n", a);

	printf("SA_STIC:\n");

        a = salloc(64,SA_STIC);
        printf("   a@%p\n", a);

        b = salloc(64,SA_STIC);
        printf("   b@%p\n", a);

	printf("SA_STPC:\n");

        for (i = 0; i < 8; i++) {
                a = salloc(64, SA_STPC);
                b = salloc(64, SA_STPC);
                printf("   a@%p b@%p\n", a, b);
        }
	
	return 0;
}

#endif
