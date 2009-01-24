/*
 * MSET: memory segment executable test
 *
 * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
 *                    Alberto Ornaghi <alor@antifork.org>
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

#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

static const char cvsid[] = "$Id: mset.c,v 1.21 2003/05/01 20:06:57 awgn Exp $";

/*
 *
 8048318:       bb af 00 00 00          mov    $0xaf,%ebx
 804831d:       53                      push   %ebx
 804831e:       53                      push   %ebx 		// garbage
 804831f:       b8 01 00 00 00          mov    $0x1,%eax
 8048324:       cd 80                   int    $0x80
 *
 */

#if (!__i386__)
#error "Only i386 arch is supported"
#endif

#define i386_EXITCODE	"\xbb\xaf\x00\x00\x00\x53\x53\xb8\x01\x00\x00\x00\xcd\x80"
#define i386_EXITVALUE	0xaf	/* antifork doh'! */

#define PAGE_SHIFT      12
#define PAGE_SIZE       (1UL << PAGE_SHIFT)
#define PAGE_MASK       (~(PAGE_SIZE-1))
#define PAGE_TEST(p,t)	( ((p) & PAGE_MASK) == ((t) & PAGE_MASK) )

#define MSET_STACK	0x1
#define MSET_BSS	0x2
#define MSET_HEAP	0x4
#define MSET_EURISTIC	0x8

/* tests */
#define TEST_STACK      0
#define TEST_BSS        1
#define TEST_HEAP       2

struct test_s {
	char *segment;		/* segment */
	void (*addr) ();	/* address */
};

char exit_bss[] = i386_EXITCODE;
char author[] = "bonelli@antifork.org";

int pid;
int options;

void *
ret_euristic(long *ret)
{
	unsigned int retval;

	printf("       __builtin_return_address(1)=0x%x\n", retval = (unsigned int) __builtin_return_address(1));
	printf("       *ret=0x%x\n", (unsigned int) *ret);

	assert(PAGE_TEST(retval, *ret));

	if ((retval != 0xdeadbeef) && (retval == *ret)) {
		printf("   ==> ret found @%p. (classic prolog) <==\n", ret);
		return ret;
	}
	ret++;

	if ((retval != 0xdeadbeef) && (retval == *ret)) {
		printf("   ==> ret found @%p. (canaries prolog) <==\n", ret);
		return ret;
	}
	printf("   ==> RET NOT FOUND. (!?!) <==\n");
	exit(-1);
}


void
test_stack(long sep)
{
	long *ret;
	char exit_stack[] = i386_EXITCODE;
	__asm__ __volatile__("movl %%ebp, %0"::"g"(ret));
	ret++;

	if (options & MSET_EURISTIC)
		ret = ret_euristic(ret);

	printf("   ==> forcing RET to 0x%x <==\n", (unsigned int) exit_stack);
	(*ret) = (long) exit_stack;
	return;
}

void
test_bss(long sep)
{
	long *ret;
	__asm__ __volatile__("movl %%ebp, %0"::"g"(ret));
	ret++;

	if (options & MSET_EURISTIC)
		ret = ret_euristic(ret);

	printf("   ==> forcing RET to 0x%x <==\n", (unsigned int) exit_bss);
	(*ret) = (long) exit_bss;
	return;
}

void
test_heap(long sep)
{
	long *ret;
	char *exit_heap;
	__asm__ __volatile__("movl %%ebp, %0"::"g"(ret));
	ret++;

	exit_heap = malloc(sizeof(i386_EXITCODE));
	if (exit_heap == NULL) {
		printf("Can't allocate memory");
		exit(-1);
	}
	memcpy(exit_heap, i386_EXITCODE, sizeof(i386_EXITCODE));

	if (options & MSET_EURISTIC)
		ret = ret_euristic(ret);

	printf("   ==> forcing RET to 0x%x <==\n", (unsigned int) exit_heap);
	(*ret) = (long) exit_heap;
	return;
}


struct test_s test_code[] = {
	{segment:"stack", addr:test_stack},
	{segment:"bss", addr:test_bss},
	{segment:"heap", addr:test_heap}
};

int
fork_exec(int i)
{
	int status = 0xffffffff;

	switch (pid = fork()) {
	case -1:
		fprintf(stderr, "-> fork() :%s\n", strerror(errno));
		exit(-1);
	case 0:		/* child */
		usleep(1000);
		test_code[i].addr(0xdeadbeef);
		break;
	default:		/* parent */
		printf("-> exec-%s test\n", test_code[i].segment);
		printf("       pid=%d: child forked!\n", pid);
		do {
			if (waitpid(pid, &status, 0) == -1) {
				if (errno != EINTR)
					return -1;
			} else
				return status;
		}
		while (1);
	}

	return -1;		/* unreachable */
}

void
exit_evaluation(int t, int r)
{
	if (WIFEXITED(r) != 0) {
		/* child exited normally */
		printf("       pid=%d: exit(0x%x)\n", pid, WEXITSTATUS(r));
		if (i386_EXITVALUE == WEXITSTATUS(r))
			printf("   >>> warning, %s is executable! <<<\n", test_code[t].segment);
		else {
			printf("   !unexpected error: WEXITSTATUS(%d)!\n",
			       WEXITSTATUS(r));
			exit(1);
		}

	} else {
		if (WIFSIGNALED(r)) {
			/* the child has been signaled */
			printf("       pid=%d: catched SIGNAL %d\n", pid, WTERMSIG(r));
			printf("   *** %s *** is probably not executable\n",
			       test_code[t].segment);
		} else {
			printf("   !unexpected error: child stopped?!\n");
			exit(1);
		}
	}
}

extern char *__progname;

void
usage()
{

	printf("%s [OPTIONS]:\n", __progname);
	printf("   -s               test stack executable only.\n");
	printf("   -b               test bss executable only.\n");
	printf("   -h               test heap executable only.\n");
	printf("   -e               euristic check for canaries.\n");
	printf("   -H               print this help.\n");
	exit(1);
}

int
main(int argc, char **argv)
{
	struct utsname buf;
	unsigned long ret;
	int i;

	uname(&buf);
	printf("Memory segment executable test: %s %s %s %s\n%s <%s>\n",
	       buf.sysname, buf.release, buf.version, buf.machine, cvsid, author);

	/* default */

	options |= (MSET_STACK | MSET_BSS | MSET_HEAP);

	while ((i = getopt(argc, argv, "sbehH")) != EOF)
		switch (i) {
		case 's':	/* stack only */
			options &= ~(MSET_HEAP | MSET_BSS);
			break;
		case 'b':	/* bss only */
			options &= ~(MSET_STACK | MSET_HEAP);
			break;
		case 'h':	/* heap only */
			options &= ~(MSET_STACK | MSET_BSS);
			break;
		case 'e':
			options |= MSET_EURISTIC;
			break;
		case 'H':
			usage();
		}

	argc -= optind;
	argv += optind;

	if (options & MSET_STACK) {
		ret = fork_exec(TEST_STACK);
		exit_evaluation(TEST_STACK, ret);
	}
	if (options & MSET_BSS) {
		ret = fork_exec(TEST_BSS);
		exit_evaluation(TEST_BSS, ret);
	}
	if (options & MSET_HEAP) {
		ret = fork_exec(TEST_HEAP);
		exit_evaluation(TEST_HEAP, ret);
	}
	return 0;
}
