/*
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
 */

#ifndef _MSTACK_H_
#define _MSTACK_H_


/* 
 * a novel circular stack: a LIFO/FIFO queues implementation.
 */

static const char cvsid[]= "$Id: mstack.h,v 1.8 2005/11/21 23:21:09 awgn Exp $";


#define __mstack_init(name,type,depth) { 							\
        .sp  = name.stack,                                                                      \
        .bp  = name.stack,                                                                      \
        .nm  = 1<<depth,                                                                        \
        .sz  = (1<<depth)*sizeof(type),                                                         \
        .mask= (1<<depth)*sizeof(type)-1,                                                       \
}       

#define __mstack_common_data(type) 								\
        type *sp;                                                                               \
        type *bp;                                                                               \
        int   nm;                                                                               \
        int   sz;                                                                               \
        int  mask;                                                                              

/*
 * stack declaration:
 *       name  = structure name.
 *       type  = typeof() elements of the stack.
 *       depth = in bit. The stack size is 2^depth-1. 
 */
	
#ifdef __ALIGNED_STACK__ 
#define MSTACK(name,type,depth)									\
struct {											\
	type  stack[1<<depth] __attribute__ ((aligned( (1<<depth) * sizeof(type))));		\
	__mstack_common_data(type);								\
} name = __mstack_init(name,type,depth)
#else
#define MSTACK(name,type,depth)                                                                 \
struct {                                                                                        \
        type  stack[1<<depth];									\
	__mstack_common_data(type);								\
} name = __mstack_init(name,type,depth) 
#endif


/* remap an address into the stack in a circular fashion. */

#define _(x)	(long)(x)
#ifdef __ALIGNED_STACK__ 
#define __mstack_rmap(base,ptr,size,mask)   	( _(base) | ( _(ptr) & _(mask) ))
#else
#define __mstack_rmap(base,ptr,size,mask)	( _(base) + ( _(ptr)+_(size)-_(base) ) % _(size) )
#endif


/*
 * push an element into the stack
 */
#define MSTACK_PUSH(name,elem) 	({ 								\
       *name.sp = elem;										\
	name.sp = (typeof(name.sp))__mstack_rmap(name.stack,name.sp+1,name.sz,name.mask);	\
      ( name.sp == name.bp ?  									\
        name.bp = (typeof(name.bp))__mstack_rmap(name.stack,name.bp+1,name.sz,name.mask), 0:1 );\
})

/* aliases */
#define MSTACK_LIFO_PUSH(name,elem)	MSTACK_PUSH(name,elem)
#define MSTACK_FIFO_PUSH(name,elem)     MSTACK_PUSH(name,elem)


/* 
 * pop an element from the stack in a LIFO fashion 
 */
#define MSTACK_POP(name)  ({                                                           		\
	if (name.sp != name.bp)									\
	name.sp = (typeof(name.sp))__mstack_rmap(name.stack, name.sp-1, name.sz, name.mask);    \
	*name.sp;										\
})
/* alias */ 
#define MSTACK_LIFO_POP(name)	MSTACK_POP(name)


/* 
 * pop(get) an element from the stack in a FIFO fashion 
 */
#define MSTACK_GET(name)  ({                                                                   	\
	typeof( name.bp) _nxt =(typeof(name.bp))__mstack_rmap(name.stack,name.bp+1,name.sz,name.mask); \
	typeof( name.bp) _prc =(typeof(name.bp))__mstack_rmap(name.stack,name.bp-1,name.sz,name.mask); \
	typeof(*name.bp) _ret =( name.bp == name.sp ? *_prc : *name.bp );			\
	name.bp = ( name.sp == name.bp ? name.bp : _nxt );					\
        _ret;                                                                               	\
}) 
/* alias */
#define MSTACK_FIFO_POP(name)	MSTACK_GET(name)


/*************************************************/

#ifdef   TEST_MSTACK
#include <stdio.h>


#define print(frm,fun)	printf("%s=" frm, #fun,fun)
#define print_stack(name)	do {		\
        printf("stack:%p\n", name.stack);	\
        printf("sp   :%p\n",    name.sp);	\
        printf("bp   :%p\n",    name.bp);	\
        printf("sz   :%d\n",    name.sz);	\
        printf("mask :0x%x\n",name.mask);	\
} while (0)


MSTACK(my_stack,int,2); /* depth = 2 bit, size = 4 */

int
main(int argc, char **argv)
{

        printf("------------------------\n");
        print_stack(my_stack);
        printf("------------------------\n");

        print("%d\n",MSTACK_PUSH(my_stack,0));
        print("%d\n",MSTACK_PUSH(my_stack,1));
        print("%d\n",MSTACK_PUSH(my_stack,2));
        print("%d\n",MSTACK_PUSH(my_stack,3));


        print("[%d]\n", MSTACK_POP(my_stack));
        print("[%d]\n", MSTACK_POP(my_stack));
        print("[%d]\n", MSTACK_POP(my_stack));
        print("[%d]\n", MSTACK_POP(my_stack));
        print("[%d]\n", MSTACK_POP(my_stack));

        printf("------------------------\n");

        print("%d\n",MSTACK_PUSH(my_stack,4));
        print("%d\n",MSTACK_PUSH(my_stack,5));
        print("%d\n",MSTACK_PUSH(my_stack,6));

        print("[%d]\n", MSTACK_POP(my_stack));
        print("[%d]\n", MSTACK_POP(my_stack));
        print("[%d]\n", MSTACK_POP(my_stack));
        print("[%d]\n", MSTACK_POP(my_stack));
        printf("------------------------\n");

        print("%d\n",MSTACK_PUSH(my_stack,1));
        print("%d\n",MSTACK_PUSH(my_stack,2));
        print("%d\n",MSTACK_PUSH(my_stack,3));

        print("[%d]\n", MSTACK_GET(my_stack));
        print("[%d]\n", MSTACK_GET(my_stack));
        print("[%d]\n", MSTACK_GET(my_stack));
        print("[%d]\n", MSTACK_GET(my_stack));
        print("[%d]\n", MSTACK_GET(my_stack));

        printf("------------------------\n");
        print_stack(my_stack);
        printf("------------------------\n");

        print("%d\n",MSTACK_PUSH(my_stack,4));
        print("%d\n",MSTACK_PUSH(my_stack,5));
        print("%d\n",MSTACK_PUSH(my_stack,6));
        print("%d\n",MSTACK_PUSH(my_stack,7));
        print("%d\n",MSTACK_PUSH(my_stack,8));


        print("[%d]\n", MSTACK_GET(my_stack));
        print("[%d]\n", MSTACK_GET(my_stack));
        print("[%d]\n", MSTACK_GET(my_stack));
        print("[%d]\n", MSTACK_GET(my_stack));
        printf("------------------------\n");

        return 0;
}

#endif
#endif /* _MSTACK_H */


