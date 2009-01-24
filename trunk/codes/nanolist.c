/*
 * $Id: nanolist.c,v 1.7 2005/07/19 17:56:05 awgn Exp $
 * nanolist- add/replace headers to local-delivery mails.
 *
 * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
 *
 * All rights reserved.
 *
 * Tested on some linux(-ldl) and BSD flavors
 *
 * Add to /etc/aliases:
 * ml-nanolist:	"|/usr/local/bin/nanolist /etc/ml-conf.so"
 *          all:        list of users...    
 *
 * note: the alias "all" is the one compiled within the ml-conf.so.
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

#include <sys/types.h>
#include <sys/wait.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sysexits.h>
#include <string.h>
#include <dlfcn.h>

#include "nanolist.h"

static const char cvsid[] = "$Id: nanolist.c,v 1.7 2005/07/19 17:56:05 awgn Exp $";

#define BUFFLEN		1024

#if defined(__OpenBSD__) && !defined(RTLD_NOW)
/* The below define is a lie since we are really doing RTLD_LAZY ...
 */
#define RTLD_NOW DL_LAZY
#endif

#if defined(__OpenBSD__)
#define SYM(x)	"_" ## x
#else
#define SYM(x)	x
#endif

#ifdef NANOTEST
char *nanotest_args[] = { "/bin/more", NULL };
#endif

int filedes[2];

/*** shared object symbols ***/
struct tag *header;
int *headerlen;
char *mailto;
char **args;


/*** strcmp_() ***/

static int
strcmp_(char *a, char *b)
{
	while (*a && *b) {
		if (*a != *b)
			break;
		a++, b++;
	}
	return (int) (*a && *b);
}

static void
load_conf(char *so)
{
	void *handle;

	/* opening the data.so */

	handle = dlopen(so, RTLD_NOW);
	if (handle == NULL)
		exit(EX_NOPERM);

	headerlen = dlsym(handle, SYM("headerlen"));
	header    = dlsym(handle, SYM("headers"));
	mailto    = dlsym(handle, SYM("mailto"));

#ifndef NANOTEST
        args      = dlsym(handle, SYM("args"));
#else
        args      = nanotest_args;
#endif

	if (header == NULL || headerlen == NULL || mailto == NULL || args == NULL)
		exit(EX_CONFIG);
}

static void
child(char **envp)
{
	close(filedes[1]);
	dup2(filedes[0], 0);
	execve(args[0], args, envp);
}

static void
parent()
{
	char buffer[BUFFLEN];
	char From[8];
	FILE *stdpipe;
	int i, l;

	close(filedes[0]);
	stdpipe = fdopen(filedes[1], "w");

	/*** 
	 *** append new headers 
	 ***/

	for (i = 0; i < *headerlen; i++) {
		if (header[i].value == NULL || header[i].mode != replace )
			continue;
		fprintf(stdpipe, "%s %s\n", header[i].tag, header[i].value);
	}

	/*** 
         *** processing headers 
         ***/

	for (l = 0; fgets(buffer, BUFFLEN, stdin) != NULL; l++) {

		/*
	         * In the case of UNIX-style mailbox delivery, the local
	         * daemon prepends a "From  sender  time_stamp" envelope
	         * header to each message, which is actually removed.
	         */
		if (l == 0 && sscanf(buffer, "%6s", From) && !strcmp(From, "From"))
			continue;

		for (i = 0; i < *headerlen && strcmp_(buffer, header[i].tag) != 0; i++);

                if (i == *headerlen )
                        goto tag_not_found;

		/*** skip replaced header ***/
		if (i != *headerlen && header[i].mode == replace )
			continue;
	
		/*** append tag to headers ***/
		if ( 	header[i].mode == append && header[i].value != NULL && header[i].tag != NULL &&
		        strstr(buffer,header[i].value) == NULL ) {
			fprintf(stdpipe, "%s %s %s", header[i].tag,header[i].value,buffer+strlen(header[i].tag)+1);
			continue;
		}

   tag_not_found:

		/*** keep orig ***/
		fprintf(stdpipe, "%s", buffer);
		if (buffer[0] == '\n')
			break;
	}

	/*** 
	 *** body 
	 ***/

	while (fgets(buffer, BUFFLEN, stdin) != NULL) {
		fprintf(stdpipe, "%s", buffer);
	}

	fclose(stdpipe);
	close(filedes[1]);
}

int
main(int argc, char **argv, char **envp)
{
	int i;

	if (argc < 2)
		exit(EX_CONFIG);

	load_conf(argv[1]);

	/*
	 * filedes[1] : writing -> parent 
	 * filedes[0] : reading -> child
	 */

	pipe(filedes);

	if ((i = fork()) == -1) {
		exit(EX_NOUSER);
	}
	if (i == 0)
		child(envp);

	/* parent */
	parent();

	/* wait for the child to terminate */
	wait(&i);

	exit(EX_OK);
}
