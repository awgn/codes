/*
 * buffy-tune: According to the ratio dirty-word/word, buffy-tune determines the 
 *             threshold suitable for choosing the memory reset method. 
 *             The threshold depends by two factors: the implementation of memset() 
 *             in the operating system and the optimization options used to compile 
 *             buffy-tune (which must be the same for buffy.c). 
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
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

static char cvsid[] = "$Id: buffy-tune.c,v 1.10 2004/07/10 13:04:28 awgn Exp $";

#define bit         >>3
#define kbit        <<7
#define mbit        <<17
#define gbit        <<27
#define kbyte       <<10
#define mbyte       <<20
#define gbyte       <<30

#define BSIZE		(1 mbyte)
#define STEP		16	
#define REPT		256	
#define DELTA		32	
		
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

void *
smalloc(size_t b)
{
	void *ret;
	if ((ret = (u_char *) malloc(b)) == NULL)
		fatal(__FUNCTION__);
	return ret;
}

struct timeval
timeval_diff(struct timeval tv1, struct timeval tv2)
{
	struct timeval diff;

	diff.tv_sec = tv2.tv_sec - tv1.tv_sec;
	diff.tv_usec = tv2.tv_usec - tv1.tv_usec;
	if (diff.tv_usec < 0) {
		diff.tv_sec--;
		diff.tv_usec += 1000000;
	}
	return diff;
}


void
usage()
{
	printf("usage: buffy-tune <chunks>\n");
	exit(0);
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	struct timeval t1, t2;
	struct timeval d1;
	int i, j, k, step;
	int sec, usec;
	int rept = REPT-DELTA;
	char *b;
	int *vo;

	if (argc>1) {
		if ( argv[1][0] == '-' && argv[1][1]== 'h' )
			usage();
		step = atoi(argv[1]);
		step = ( step == 0 ? STEP : step );
	}
	else
		step = STEP;

        printf("[!] Tune-buffy. %s\n", cvsid);
        printf("[*] Allocating base buffer (repeating test %d times)...\n", REPT);

	b = smalloc(BSIZE);
	vo = (int *) b;
	
	k = 0;
	do {
		if ( (rept += DELTA) != REPT )
			printf("[!] Round error. Resuming...\n");
		k++;
		sec = usec = 0;
		for (j = 0; j < step; j++) {
			printf("    Step %d/%d: ", j+1, step);
			fflush(stdout);

			gettimeofday(&t1, NULL);
			for (i = 0; i < rept ; i++)
				memset(b, 0, BSIZE);

			gettimeofday(&t2, NULL);
			d1 = timeval_diff(t1, t2);
			printf("sec=%d usec=%d\n", (int)d1.tv_sec,(int)d1.tv_usec);
			sec += d1.tv_sec;
			usec += d1.tv_usec;
		}
		printf("[*] Elapsed time -> sec:%d usec:%d\n", sec, usec);
	}
	while (sec % step && k < 10);

	if ( k == 10 )
		fatal("[%] Unrecoverable round error");

	printf("[*] Mean time -> sec:%d usec:%d\n", sec/step, usec/step);

		
	for (k=1; k <= step; k++) {
		int sup = (BSIZE>>2)*k/step;
		printf("    Step %d/%d: ",k,step);
		fflush(stdout);

		gettimeofday(&t1, NULL);

		for (j=0; j < rept ; j++) {
			for (i=0;i < sup ; i++)	
				vo[i]=0;	
		}
		gettimeofday(&t2, NULL);
		d1 = timeval_diff(t1, t2);	
		printf("sec=%d usec=%d\n", (int)d1.tv_sec,(int)d1.tv_usec);

		if ( d1.tv_sec > (sec/step) || ( (d1.tv_sec == sec/step) && (d1.tv_usec > (usec/step)) ) )
			break;	
	}

	printf("[!] Add the following defines to buffy.h\n");
	printf("#define MEMSET_THR	%d	/* threshold %d/%d */\n",k,k,step);
	printf("#define MEMSET_TOT	%d 	/* total chunks */\n",step); 

	return 0;
}
