/*
 * ringlib- 
 *
 * Copyright (c) 2005 Nicola Bonelli <bonelli@antifork.org>
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

#ifndef RING_H
#define RING_H

#ifdef __KERNEL__

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>

#define alloc(x)	kmalloc(x, GFP_KERNEL)
#define freed(x)	kfree(x)

#else

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define alloc(x)	malloc(x)
#define freed(x)	free(x)

#endif


/* 
 * ring element
 */ 
struct r_elem {
        int val;
};


typedef struct {
	struct r_elem	*r_ring;
	size_t		r_size;		/* size of the ring */
	int		r_first;	/* index of the first element */
	int		r_last;		/* index of the last  element */
	int		r_nelem;
} ring_t;


/* prototypes */

ring_t  * ring_alloca(size_t );
void ring_free(ring_t *);
struct r_elem *ring_pop(ring_t *);
int ring_push(ring_t *,struct r_elem *);


#endif /* RING_H */
