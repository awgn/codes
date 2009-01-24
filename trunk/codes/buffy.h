/*
 * buffy: Large buffer menagement framework. 
 *
 * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
 *
 * All rights reserved.
 *
 * Tested on some linux and BSD flavors
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

#ifndef BUFFY_H
#define BUFFY_H

#include <sys/types.h>
 
#define bit  	>>3
#define kbit    <<7
#define mbit    <<17
#define gbit    <<27
#define kbyte   <<10
#define mbyte   <<20
#define gbyte   <<30 

#define REG_BS(r,b)  ( r[b>>3] |=   1<<(b&7) )
#define REG_BR(r,b)  ( r[b>>3] &= ~ 1<<(b&7) )
#define REG_BT(r,b)  ( r[b>>3]  &   1<<(b&7) )
#define REG_BN(r,b)  ( r[b>>3] ^=   1<<(b&7) )
#define REG_OFF(r,b) ( r[b>>3] )

typedef struct {
        u_char *buff;           /* effective buffer */
        u_char *bitmap;         /* bitmap to cover dirty words */
        u_long **dirty; 	/* pointers to dirty words*/
        u_int b_len;            /* sizeof(buff)  */
        u_int d_len;            /* sizeof(dirty) */
        u_int max;              /* actual number of dirty words */
} SEG;

#ifdef BUFFY_LIB_C

#ifdef EBUG
#define DEBUG(format, ...) do {                         \
fprintf (stderr, "   %s:", __PRETTY_FUNCTION__);        \
fprintf (stderr, format, ## __VA_ARGS__);               \
} while (0)
#else
#define DEBUG(f,arg...) do {} while (0)
#endif

#define CEIL(x,y)       ( (x)/(y)+( (x)%(y) ? 1 : 0 ) )
#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )

/* Add here the buffy-tune defines */

#define MEMSET_THR      7       /* threshold 7/16 */
#define MEMSET_TOT      16      /* total chunks */

#endif
#endif /* BUFFY_H */

