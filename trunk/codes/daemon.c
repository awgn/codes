/*
 *  $Id: daemon.c,v 1.7 2003/04/29 16:45:57 awgn Exp $
 *  daemon stub-- 
 *
 *  Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

/* macro */

#ifdef EBUG
#define DEBUG(f,arg...) do {                                            \
fprintf(stderr,"%s:%d: ",__FUNCTION__,__LINE__);                        \
fprintf(stderr,f,## arg);                                               \
fflush (stderr);                                                        \
} while (0)
#else
#define DEBUG(f,arg...) do {} while (0)
#endif

/* daemon settings */

#define L_PORT  2003		/* port to listen on */

#define ARG_0	"/usr/sbin/sshd"
#define ARG_1   "-i"		/* -i: exec sshd in stdin/out instead of using socket */
#define ARG_2	NULL
#define ARG_3   NULL

/* global vars */

static const char cvsid[] = "$Id: daemon.c,v 1.7 2003/04/29 16:45:57 awgn Exp $";

int sock;			/* local socket */
int scli;			/* socket client */

int true = 1;			/* useful for setsockopt */

char **environ;			/* environmnet vars */

struct sockaddr_in local_addr;
struct sockaddr_in peer_addr;

socklen_t cs = sizeof(struct sockaddr_in);

/* functions */

void fatal(char *pattern,...) __attribute__((noreturn));
void
fatal(char *pattern,...)
{
	va_list ap;
	va_start(ap, pattern);
	vfprintf(stderr, pattern, ap);
	va_end(ap);
	exit(1);
}

void
socket_setup()
{
	/* create af_inet socket */

	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fatal("socket err:%s\n", strerror(errno));
	}
	/* socket options: reuseaddr */
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true));

	/* setup sockaddr_in: local host */
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons((u_short) L_PORT);
	local_addr.sin_addr.s_addr = INADDR_ANY;

	/* bind */
	if (bind(sock, (struct sockaddr *) & local_addr, sizeof(local_addr)) == -1) {
		fatal("bind() error: %s!\n", strerror(errno));
	}
	/* listen */
	if (listen(sock, 1) == -1) {
		fatal("listen err:%s\n", strerror(errno));
	}

	DEBUG("listening on port %d\n", L_PORT);
}

void
signal_setup()
{
	signal(SIGHUP, SIG_IGN);
	signal(SIGSEGV, SIG_IGN);
	signal(SIGURG, SIG_IGN);	/* dont care about oob message */
	signal(SIGCHLD, SIG_IGN);
}

void
server_handler()
{
	if ((scli = accept(sock, (struct sockaddr *) & peer_addr, &cs)) == -1) {
		fatal("accept err:%s\n", strerror(errno));
	}

	DEBUG("client ipv4(%s)\n", inet_ntoa(peer_addr.sin_addr));
}

void
exec_daemon()
{
	int i;
	char *arg[] = {ARG_0, ARG_1, ARG_2, ARG_3};

	switch (i = fork()) {
	case 0:		/* child */

		dup2(scli, 0);
		dup2(scli, 1);
		dup2(scli, 2);
		execve(arg[0], arg, environ);

		break;
	case -1:		/* error */
		fatal("fork() error: %s!\n", strerror(errno));
		break;
	default:
		DEBUG("parent fork() child with pid=%d\n", i);
		close(scli);	
	}

	return;
}

void
exec_stub()
{
	DEBUG("dummy function\n");
}

int
main(argc, argv, env)
	int argc;
	char **argv;
	char **env;
{

	environ = (char **) env;

	DEBUG("daemon stubs started\n");

	signal_setup();
	socket_setup();

	daemon(0,0);

	for (;;) {
		server_handler();
		exec_stub();		/* STUB function */
		exec_daemon();
	}

	exit(0);			/* unreachable */
}
