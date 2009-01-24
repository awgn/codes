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

static const char cvsid[]= "$Id: ring.c,v 1.1 2005/02/10 13:09:33 awgn Exp $";

#include "ring.h"

/* 
 * allocate a ring
 */
	
ring_t	*
ring_alloca(size_t size)
{
	ring_t *r;

	r = alloc(sizeof(ring_t));
	if (r==NULL)
		goto err_b;

	r->r_size  = size;
	r->r_first = 0;
	r->r_last  = 0;
	r->r_nelem = 0;

	r->r_ring  = alloc(size*sizeof(struct r_elem));
	if (r->r_ring == NULL)
		goto err_a;

	return r;

    err_a:
	freed(r);	

    err_b:
	return NULL;
}


/*
 * free the ring
 */
void
ring_free(ring_t *r)
{
	freed(r->r_ring);
	freed(r);
}


/*
 *  pop an element from the ring, in FIFO fashion.
 *  return: the pointer to the first element, NULL in case of empty ring
 *	
 */
struct r_elem *ring_pop(ring_t *r)
{
	struct r_elem *ret;

	if (r->r_first == r->r_last && r->r_nelem == 0 )
		return NULL;		/* ring empty */

	ret = &r->r_ring[r->r_last++];

	r->r_last %= r->r_size;
	r->r_nelem--;

	return ret;
}


/*
 *  push the element into the ring
 */
int ring_push(ring_t *r,struct r_elem *elem)
{
	if (r->r_nelem!=0 && r->r_first == r->r_last) /* ring overrun */
		return -1;
		
	memcpy ( &r->r_ring[r->r_first], elem, sizeof(struct r_elem));

	r->r_nelem++;
	r->r_first++;
	r->r_first %= r->r_size;

	return 0;
}


#ifdef RING_TEST
#define eval(x,y)	( x != NULL ? x->y : -1 ) 
#define dump_pop(r)	printf("<=   pop: elem @%p -> %x\n",r, eval(r,val))
#define dump_push(r,k)	printf("=>   push(%d)=%d\n",k.val, ring_push(r,&k))	
#define dump_state(r)	printf("[*]  r_first=%d, r_last=%d, r_nelem=%d\n",r->r_first,r->r_last,r->r_nelem)

int
main(argc,argv)
	int argc;
	char **argv;
{
	struct r_elem k, *p;
	ring_t	*my_ring;
	

	my_ring = ring_alloca(3);
	
	p = ring_pop(my_ring);
	dump_pop(p);
	dump_state(my_ring);

	k.val = 1;
	dump_push(my_ring,k);
	dump_state(my_ring);

	p = ring_pop(my_ring);
	dump_pop(p);
	dump_state(my_ring);

        p = ring_pop(my_ring);
	dump_pop(p);
	dump_state(my_ring);

	k.val = 1;
	dump_push(my_ring,k);
	dump_state(my_ring);

	k.val = 2;
	dump_push(my_ring,k);
	dump_state(my_ring);

        p = ring_pop(my_ring);
	dump_pop(p);
	dump_state(my_ring);

        p = ring_pop(my_ring);
	dump_pop(p);
	dump_state(my_ring);

        p = ring_pop(my_ring);
	dump_pop(p);
	dump_state(my_ring);

        k.val = 1;
	dump_push(my_ring,k);
        dump_state(my_ring);
        k.val = 1;
	dump_push(my_ring,k);
        dump_state(my_ring);
        k.val = 1;
	dump_push(my_ring,k);
        dump_state(my_ring);
        k.val = 1;
	dump_push(my_ring,k);
        dump_state(my_ring);
        k.val = 1;
	dump_push(my_ring,k);
        dump_state(my_ring);

        p = ring_pop(my_ring);
        dump_pop(p);
        dump_state(my_ring);
        p = ring_pop(my_ring);
        dump_pop(p);
        dump_state(my_ring);
        p = ring_pop(my_ring);
        dump_pop(p);
        dump_state(my_ring);
        p = ring_pop(my_ring);
        dump_pop(p);
        dump_state(my_ring);

	return 0;
}

#endif

