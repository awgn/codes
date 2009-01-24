/*
 * buffy: large buffer menagement framework.
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

#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static char cvsid[] __attribute__((unused)) = "$Id: buffy.c,v 1.15 2004/07/10 13:04:29 awgn Exp $";

#define BUFFY_LIB_C
#include "buffy.h"

static void fatal(char *pattern,...) __attribute__((noreturn));
static void
fatal(char *pattern,...)
{
	va_list ap;
	char *mg;

	va_start(ap, pattern);
	vasprintf(&mg, pattern, ap);
	va_end(ap);
	perror(mg);
	abort();
}

#define FNV32_prime 16777619UL
#define FNV32_init  2166136261UL

static u_long
fnv(u_char * p, int s)
{
	unsigned long h = FNV32_init;
	int i = 0;

	for (; i < s; i++) {
		h = h * FNV32_prime;
		h = h ^ p[i];
	}

	return h;
}


static void *
smalloc(size_t b)
{
	void *ret;
	if ((ret = (u_char *) malloc(b)) == NULL)
		fatal(__FUNCTION__);
	return ret;
}


int
reg_alloc(SEG * r, size_t byte)
{
	byte  = (byte+3) & ~(3UL);	/* multiple of 32 bit */

	r->b_len = byte;	/* buffer lenght */
	r->d_len = CEIL((byte >> 2) * MEMSET_THR, MEMSET_TOT);	/* byte/4 *
								 * MEMSET_THR/MEMSET_TOT */

	r->buff = (u_char *) smalloc(r->b_len);
	r->bitmap = (u_char *) smalloc(CEIL(byte, 32));	/* 1 bit maps 32 bit */
	r->dirty = (u_long **) smalloc(r->d_len << 2);
	r->max = -1;

	DEBUG("buff_len=%d dirty_len=%d bitmap_size=%d (bytes)\n", r->b_len, r->d_len, CEIL(byte, 32));
	return byte;
}


int
reg_free(SEG *r)
{
	if( r->b_len == 0 || r->d_len != CEIL((r->b_len >> 2) * MEMSET_THR, MEMSET_TOT) )
		return -1;
 
	free (r->buff);
	free (r->dirty);
	free (r->bitmap);

	r->buff = (u_char *)NULL;
	r->dirty = (u_long **)NULL;
	r->bitmap = (u_char *)NULL;
	r->b_len = r->d_len = r->max = -1;
	
	return 0;
}


int
reg_clear(SEG * r)
{
	if (r->max < r->d_len) {
		/* word patch clear */

		register int max = r->max;
		register u_long **ptr = r->dirty;
		//register u_long *v = (u_long *) r->buff;
		int i;

		DEBUG("-> word_patch()\n");
		for (i = 0; i < max; i++) {
			DEBUG("-> word_patch(): buffer[%p]\n", ptr[i]);
			*(ptr[i]) = 0L;
		}
	} else {
		/* clear buffer */
		DEBUG("-> memset()\n");
		memset(r->buff, 0, r->b_len);
	}

	/* clear bitmap */
	memset(r->bitmap, 0, CEIL(r->b_len, 32));

	/* clear dirty array */
	r->max = 0;
	return 0;
}


int
reg_isclear(SEG * r)
{
	u_long *v = (u_long *) r->buff;
	u_long ret = 0L;
	int i;

	for (i = 0; i < (r->b_len >> 2); i++) {
		ret |= v[i];
	}
	if (!ret) {
		/* clear bitmap */
		memset(r->bitmap, 0, CEIL(r->b_len, 32));

		/* clear dirty array */
		r->max = 0;
	}
	DEBUG("is_clear(): ->%d\n", !ret);
	return !ret;
}

/*** bit ***/

#ifdef __GNUC__
__inline
#endif
u_long
reg_bit_set(SEG * r, int i)
{
	register u_long *v = (u_long *) r->buff;
	register u_char p;

	if (r->max < r->d_len && (p = REG_OFF(r->bitmap, i >> 5), REG_BS(r->bitmap, i >> 5), p != REG_OFF(r->bitmap, i >> 5)))
		r->dirty[r->max++] = v+(i>>5);

	return v[i >> 5] |= (1 << (i & 31L));
}


#ifdef __GNUC__
__inline
#endif
u_long
reg_bit_not(SEG * r, int i)
{
	register u_long *v = (u_long *) r->buff;
	register u_char p;

	if (r->max < r->d_len && (p = REG_OFF(r->bitmap, i >> 5), REG_BS(r->bitmap, i >> 5), p != REG_OFF(r->bitmap, i >> 5)))
		r->dirty[r->max++] = v+(i>>5);

	return v[i >> 5] ^= (1 << (i & 31L));
}


#ifdef __GNUC__
__inline
#endif
u_long
reg_bit_reset(SEG * r, int i)
{
	register u_long *v = (u_long *) r->buff;
	return v[i >> 5] &= ~(1 << (i & 31L));
}


#ifdef __GNUC__
__inline
#endif
u_long
reg_bit_test(SEG * r, int i)
{
	register u_long *v = (u_long *) r->buff;
	return v[i >> 5] & (1 << (i & 31L));
}

/*** byte ***/

#ifdef __GNUC__
__inline
#endif
u_char
reg_byte_get(SEG * r, int i)
{
	return r->buff[i];
}


#ifdef __GNUC__
__inline
#endif
u_char
reg_byte_set(SEG * r, int i, char b)
{
	register u_long *v = (u_long *) r->buff;
	register u_char p;

	if (r->max < r->d_len && (p = REG_OFF(r->bitmap, i >> 2), REG_BS(r->bitmap, i >> 2), p != REG_OFF(r->bitmap, i >> 2)))
		r->dirty[r->max++] = v+(i>>2);

	return r->buff[i] = b;
}

/*** word ***/

#ifdef __GNUC__
__inline
#endif
u_long
reg_word_get(SEG * r, int i)
{
	u_long *v = (u_long *) r->buff;
	return v[i];
}


#ifdef __GNUC__
__inline
#endif
u_long
reg_word_set(SEG * r, int i, u_long w)
{
	register u_long *v = (u_long *) r->buff;
	register u_char p;

	if (r->max < r->d_len && (p = REG_OFF(r->bitmap, i), REG_BS(r->bitmap, i), p != REG_OFF(r->bitmap, i)))
		r->dirty[r->max++] = v+i;

	return v[i] = w;
}


#ifdef __GNUC__
__inline
#endif
u_long
reg_fnv(SEG * r)
{
	return fnv(r->buff, r->b_len);
}


#ifdef __GNUC__
__inline
#endif
int
reg_cmp(SEG * r, SEG * t)
{
	if (r->b_len != t->b_len)
		return 0xdefaced;
	return memcmp(r->buff, t->buff, MIN(r->b_len, t->b_len));
}


#ifdef TEST

/*
 * compile with: gcc buffy.c -DTEST -DEBUG -o buffy-test
 */

int
main()
{
	SEG x;
	int i;

	reg_alloc(&x, 1 mbyte);
	reg_clear(&x);
	reg_isclear(&x);

	printf("set bit.... ");
	fflush(stdout);
	reg_bit_set(&x, 512);

	if (reg_bit_test(&x, 512))
		printf("-> set\t\t(ok)\n");
	else
		fatal("reg_bit_set/get error");

	printf("set byte... ");

	reg_byte_set(&x, 128, 'A');

	if (reg_byte_get(&x, 128) == 'A')
		printf("-> byte=%c\t\t(ok)\n", reg_byte_get(&x, 128));
	else
		fatal("reg_byte_set/get error");

	printf("set word... ");
	reg_word_set(&x, 64, 0xcafebabe);

	if (reg_word_get(&x, 64) == 0xcafebabe)
		printf("-> word=0x%lx\t(ok)\n", reg_word_get(&x, 64));
	else
		fatal("reg_word_set/get error");

	reg_isclear(&x);

	printf("hash=0x%lx\n", reg_fnv(&x));
	reg_clear(&x);

	printf("hash=0x%lx\n", reg_fnv(&x));
	reg_isclear(&x);

	printf("reg_bit_set() cicle...\n");
	for (i = 0; i < 100; i += 5)
		reg_bit_set(&x, i);

	printf("Comparing registers... ");
	if (reg_cmp(&x, &x) == 0)
		printf("->\t(ok)\n");
	else
		fatal("reg_cmp error");

	reg_clear(&x);

	printf("reg_free()= %d ->\t\t(ok)\n", reg_free(&x));
	printf("reg_free()=%d ->\t\t(ok)\n", reg_free(&x));

	return 0;
}
#endif
