/*
 * radix bytes superb sorter.  complexity: len*3*N. 
 * 
 * Copyright (c) 2002 Nicola Bonelli <bonelli@antifork.org>
 *                              NaGA <crwm@freemail.it> <spam> ettercap elite++ </spam>
 *
 *
 * All rights reserved.
 * 
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
 *      array
 *      rad_t*
 *   +----------+                struct rad_t
 *   |          |---------->     {
 *   +----------+                  char *str; ----------------->[|||||||||\0]
 *   |          |                  ... 
 *   +----------+                  ...
 *   |          |                  ... 
 *   +----------+                 }
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CVSID   	"$Id: radix_superb.c,v 1.3 2003/10/25 13:56:01 awgn Exp $"
#define VERSION		"0.1"

#define RAD_LONG 	0
#define RAD_SHORT	1
#define RAD_STR  	2

#define EXDONT_FREE	0
#define EX_FREE		1

typedef unsigned char byte_t;


#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )


#define OFF(p,off)       (void *)p+off

#define ARR4(p,i)       (((long  *)p)[i])
#define ARR2(p,i)       (((short *)p)[i])
#define ARR1(p,k)       (((char  *)p)[k])

#define EVAL(p)          *(long *)(p)

#define RADN(s,i)	      EVAL(OFF(ARR4(s,i),off))
#define RADC(s,i,j)     ARR1 (EVAL(OFF(ARR4(s,i),off)),j)

void
c_radix (int byte, long n, void *s, void *d, unsigned long off)
{
    long          count[256];
    long          index[256];
    int           i;

    memset (count, 0, sizeof (count));

    for (i = 0; i < n; i++)
	count[(unsigned char) RADC (s, i, byte)]++;

    index[0] = 0;
    for (i = 1; i < 256; i++)
	index[i] = index[i - 1] + count[i - 1];

    for (i = 0; i < n; i++)
	*((long **) d + index[(unsigned char) RADC (s, i, byte)]++) = *((long **) s + i);

}


void
n_radix (int byte, long n, void *s, void *d, unsigned long off)
{
    long          count[256];
    long          index[256];
    int           i;

    memset (count, 0, sizeof (count));

    for (i = 0; i < n; i++)
	count[((RADN (s, i)) >> (byte * 8)) & 0xff]++;

    index[0] = 0;
    for (i = 1; i < 256; i++)
	index[i] = index[i - 1] + count[i - 1];

    for (i = 0; i < n; i++)
	*((long **) d + index[((RADN (s, i)) >> (byte * 8)) & 0xff]++) = *((long **) s + i);

}

#define radix_sort(p,q,n,lev,tag,mode) {		\
size_t off;						\
							\
	off = ( (void *)&(p[0]->tag) - (void *)p[0] );	\
							\
	switch(mode)					\
	{						\
           case RAD_LONG:				\
             n_radix (0, n, p, q, off);			\
	     n_radix (1, n, q, p, off);			\
	     n_radix (2, n, p, q, off);			\
             n_radix (3, n, q, p, off);			\
             break;					\
           case RAD_SHORT:				\
             n_radix (0, n, p, q, off);			\
             n_radix (1, n, q, p, off);			\
           case RAD_STR:				\
		{					\
		int i;					\
		i = (lev|1);				\
		for (; i > -1 ; i-=2)			\
		{					\
		c_radix (i,  n, p, q, off);		\
		c_radix (i-1,n, q, p, off);		\
		}					\
		}					\
             break;					\
	}						\
}


char         *
extend_string (char *r, int len, int mode)
{
    char         *ret;

    ret = (char *) calloc (len + 1, 1);
    strncpy (ret, r, len);
    if (mode)
	free (r);

    return ret;
}

/* user */

typedef struct
{
    char         *str;
    int           len;
    int           id;
}
rad_t;


rad_t        *array[10];
rad_t        *copy[10];




#define SUPERB_SIZE	1000000	/* 1 million of element */

char         *
rand_string (int len)
{
    char         *ret;
    char         *c;
    int          *i;
    int           j;

    ret = (char *) malloc (len + 1);
    i = (int *) ret;

    for (j = 0; j < (len >> 2); j++)
	*i++ = (rand () | 0x60606060);

    c = (char *) i;

    for (j = 0; j < (len & 3); j++)
	*c++ = ((rand () & 0xff) | 0x60);

    *(ret + len) = 0;

    return ret;

}

#define PUSH_STR(v,i,s) { v[i]->str=s ; v[i]->len = strlen(s); }
#define PRINT_TAB(p,n)  for (i=0;i<n;i++) printf("%s %d\n",p[i]->str,p[i]->id)

int
main (int argc, char **argv)
{
    int           i;
    int           max = 0;

    for (i = 0; i < 10; i++)
	array[i] = (rad_t *) malloc (sizeof (rad_t));

    PUSH_STR (array, 0, "maria");
    PUSH_STR (array, 1, "luisa");
    PUSH_STR (array, 2, "simona");
    PUSH_STR (array, 3, "samy");
    PUSH_STR (array, 4, "kartika");
    PUSH_STR (array, 5, "jada");
    PUSH_STR (array, 6, "emma");
    PUSH_STR (array, 7, "naomi");
    PUSH_STR (array, 8, "amanda");
    PUSH_STR (array, 9, "zoe");

    array[0]->id = 45;
    array[1]->id = 5;
    array[2]->id = 123;
    array[3]->id = 12;
    array[4]->id = 89;
    array[5]->id = 1;
    array[6]->id = 5;
    array[7]->id = 8;
    array[8]->id = 11;
    array[9]->id = 2;


    radix_sort (array, copy, 10, 5, id, RAD_LONG);


    PRINT_TAB (array, 10);

    for (i = 0; i < 10; i++)
	max = MAX (max, array[i]->len);

    printf ("Max len:%d\n", max);

    printf ("--------------------\n");


    for (i = 0; i < 10; i++)
	array[i]->str = extend_string (array[i]->str, 10, EXDONT_FREE);

    radix_sort (array, copy, 10, 10, str, RAD_STR);

    PRINT_TAB (array, 10);

    printf ("--------superb---------\n");

    {
	rad_t        *sta[SUPERB_SIZE];
	rad_t        *end[SUPERB_SIZE];

	int           l;

	printf ("loading %d rand strings.. ", SUPERB_SIZE);
	fflush (stdout);

	for (l = 0; l < SUPERB_SIZE; l++)
	    {
		sta[l] = (rad_t *) malloc (sizeof (rad_t));

		sta[l]->str = rand_string (8);
		sta[l]->len = 8;
		sta[l]->id = 0;

	    }

	printf (" done\n");

	printf ("extending string... ");
	fflush (stdout);

	for (l = 0; l < SUPERB_SIZE; l++)
	    sta[l]->str = extend_string (sta[l]->str, 8, EX_FREE);

	printf (" done\n");

	printf ("Sorting... ");
	fflush (stdout);

	radix_sort (sta, end, SUPERB_SIZE, 8, str, RAD_STR);

	printf ("done\n");

	// PRINT_TAB(sta,100);

    }

    return 0;
}
