/*
 * usr8550 router -- unix framework/driver
 *
 * Copyright (c) 2002 Bonelli Nicola <bonelli@antifork.org>
 *
 * Tested on: Linux(gentoo), OpenBSD, FreeBSD, NetBSD.
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
 */

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <stdarg.h>
#include <sysexits.h>
#include <errno.h>
#include <err.h>

static const char cvsid[] = "$Id: usr8550.c,v 1.4 2004/08/18 22:46:25 awgn Exp $";
static const char copyright[] = "Copyright (c) 2002 Bonelli Nicola <bonelli@antifork.org>";

#define AUTHOR        	"Copyright (c) 2002 Bonelli Nicola <bonelli@antifork.org>"
#define VERSION       	"0.24"
#define CVSID         	"$Id: usr8550.c,v 1.4 2004/08/18 22:46:25 awgn Exp $"
#define PASSWORD	"12345"
#define CLOSECOMM	"@close"
#define HOME            "home"
#define LF              "\n"

#define TELNET_PORT	23
#define CONNECTWAIT	5	/* timeout */

/*
 * commands: ==> add your commands here <==
 */

#define C_RAW		0
#define C_GETIP		1
#define C_VERSION    	2
#define C_DNSRELAY	3
#define C_DEFROUTE	4
#define C_UPTIME	5
#define C_REBOOT	6
#define C_PNAT		7
#define C_MONITOR	8	

/*
 * macro
 */

#define ON_ERROR(x,o,y,f,arg...) do { if (x o y) fatal( f,##arg); } while (0)
#define PRINTF(f,arg...) do { if (options.verbose) fprintf(stderr,f,##arg);} while (0)
#define SET(x) (options.x=1)

/*
 * prototypes
 */

#if defined(__STDC__) || defined(__cplusplus)
#define _P(s) s
#else
#define _P(s) ()
#endif

/* usr8550.c */
void fatal _P((char *pattern,...)) __attribute__((noreturn));
ssize_t swrite _P((int fd, const void *buf, size_t count));
ssize_t swrite_echo _P((int fd, const void *buf, size_t count));
ssize_t sread _P((int fd, char *buf, size_t count));
ssize_t atomicio _P((ssize_t(*f) (), int fd, void *_s, size_t n));
void print_promptless _P((char *b));
void create_socket _P((void));
void create_sockaddr_in _P((void));
int connect_host _P((void));
unsigned long getlongbyname _P((char *host));
void usage _P((void));
void login_procedure _P((void));
void logout_procedure _P((void)) __attribute__((destructor));

void c_raw _P((void));
void c_getip _P((void));
void c_version _P((void));
void c_dnsrelay _P((void));
void c_defroute _P((void));
void c_uptime _P((void));
void c_reboot _P((void));

#undef _P

/*
 * global vars
 */

struct {
	int verbose:1;
	int pass:1;
}      options;

typedef struct {
	void (*handler) ();
}      item;

struct sockaddr_in raddr;

char buff_in[1024];
char echo_in[1024];
char *router_host;
char *password;
char *raw_command;

int sock = -1;
int comm = -1;


/*
 * functions
 */

void fatal(char *pattern,...) __attribute__((noreturn));
void
fatal(char *pattern,...)
{
        va_list ap;

        va_start(ap, pattern);

        if (errno)
                verr(EX_SOFTWARE,pattern,ap);
        else
                verrx(EX_SOFTWARE,pattern,ap);
}


/*
 * ensure all of data on socket comes through. f==read || f==write
 */

ssize_t
atomicio(f, fd, _s, n)
ssize_t(*f) ();
	int fd;
	void *_s;
	size_t n;
{
	char *s = _s;
	ssize_t res, pos = 0;

	while (n > pos) {
		res = (f) (fd, s + pos, n - pos);
		switch (res) {
		case -1:
			if (errno == EINTR || errno == EAGAIN)
				continue;
			else
				fatal("atomicio(): %s\n", strerror(errno));
		case 0:
			return res;
		default:
			pos += res;
		}
	}
	return pos;
}

/*
 *    bsd_select: a portable standard select() which leaves timeout unchanged --
 *
 *    select() should probably have been designed to return the time remaining
 *    from the original timeout, if any, by modifying the time value in place.
 *    Even though some systems stupidly act in this different way, it is un-
 *    likely this semantic will ever be commonly implemented, as the change
 *    causes massive source code compatibility problems.  Furthermore, recent
 *    new standards have dictated the current behaviour. In general, due to
 *    the existence of those brain-damaged non-conforming systems, it is unwise
 *    to assume that the timeout value will be unmodified by the select() call,
 *    and the caller should reinitialize it on each invocation.
 */

int
bsd_select(n, readfds, writefds, exeptfds, timeout)
	int n;
	fd_set *readfds;
	fd_set *writefds;
	fd_set *exeptfds;
	struct timeval *timeout;
{
	struct timeval safe;
	int res;

	if (timeout != NULL) {
		safe.tv_sec = timeout->tv_sec;
		safe.tv_usec = timeout->tv_usec;
		res = select(n, readfds, writefds, exeptfds, timeout);
		timeout->tv_sec = safe.tv_sec;
		timeout->tv_usec = safe.tv_usec;
	} else
		res = select(n, readfds, writefds, exeptfds, timeout);

	return res;
}

/*
 * write message+LF to socket
 */

ssize_t
swrite(fd, buf, count)
	int fd;
	const void *buf;
	size_t count;
{
	ssize_t res;
	res = atomicio(write, fd, (void *) buf, count);
	res += atomicio(write, fd, LF, 1);
	return res;
}

/*
 *  write message+LF to socket, expecting the echo (ala telnet)
 */

ssize_t
swrite_echo(fd, buf, count)
	int fd;
	const void *buf;
	size_t count;
{
	ssize_t res;
	res = atomicio(write, fd, (void *) buf, count);
	res += atomicio(write, fd, LF, 1);

	atomicio(read, fd, echo_in, res);
	if (memcmp(buf, echo_in, res - 1) != 0)
		fatal("write error on socket: echo!?");

	return res;
}

/*
 * read a multiple-line until data is available on socket or timeout
 */

ssize_t
sread(fd, buf, count)
	int fd;
	char *buf;
	size_t count;
{
	fd_set fdset;
	ssize_t res, pos;
	struct timeval timeout = {0, 50000};

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	pos = read(fd, buf, count);
	while (bsd_select(fd + 1, &fdset, NULL, NULL, &timeout) == 1) {
		if ((res = read(fd, buf + pos, count - pos)) == 0)
			break;
		pos += res;
	}

	buf[pos] = '\0';
	return pos;
}

/*
 * print multiple-line on stdout, discarding the prompt (if present)
 */

void
print_promptless(b)
	char *b;
{
	char t[4];
	char *r;

	r = strrchr(b, '\n');
	if (r == NULL)
		goto end;
	if (sscanf(r + 1, "%*d.%*d.%*d.%*d%4[>]", t) == 0)
		goto end;
	*r = 0;
end:
	printf("%s\n", b);
	return;
}

/*
 * create an AF_INET stream socket
 */

void
create_socket()
{
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fatal("cannot create socket");
	}
}

/*
 * fill the sockaddr_in structure in order to connect the socket
 */

void
create_sockaddr_in()
{
	raddr.sin_family = AF_INET;
	raddr.sin_port = htons((u_short) TELNET_PORT);
	raddr.sin_addr.s_addr = getlongbyname(router_host);
}

int
connect_host()
{
	int res;
	alarm(CONNECTWAIT);
	res = connect(sock, (struct sockaddr *) & raddr, sizeof(raddr));
	alarm(0);
	ON_ERROR(res, ==, -1, "connect():%s\n", strerror(errno));
	return res;
}

unsigned long
getlongbyname(host)
	char *host;
{
	struct in_addr addr;
	struct hostent *host_ent;

	if ((addr.s_addr = inet_addr(host)) == -1) {

		if ((host_ent = gethostbyname(host)) == NULL) {
			fatal("gethostbyname() or inet_addr() err:%s", strerror(errno));
		}
		memcpy((char *) &addr.s_addr, host_ent->h_addr, host_ent->h_length);
	}
	return addr.s_addr;
}

void
usage()
{
	fprintf(stderr, "Usage: usr [options] router-host\n");
	fprintf(stderr, " Commands\n");
	fprintf(stderr, "   -i                        print public ip\n");
	fprintf(stderr, "   -j                        print firmware version\n");
	fprintf(stderr, "   -d                        print dnsrelay config\n");
	fprintf(stderr, "   -g                        print defaulroute\n");
	fprintf(stderr, "   -u                        print uptime\n");
        fprintf(stderr, "   -n                        print nat inbound list\n");
	fprintf(stderr, "   -r                        reboot router\n");
	fprintf(stderr, "   -w \"commmand\"             raw command\n");
	fprintf(stderr, "   -m                        monitor\n");
	fprintf(stderr, " Others\n");
	fprintf(stderr, "   -p pass                   Use this pass to login router\n");
	fprintf(stderr, "   -v                        Verbose; display verbose messages.\n");
	fprintf(stderr, "   -V                        Display version number.\n");
	fprintf(stderr, "   -h                        Print this help\n");
	exit(0);
}

/*
 * login procedure
 */

void
login_procedure()
{
	create_socket();
	create_sockaddr_in();
	connect_host();

	sread(sock, buff_in, 1024);
	ON_ERROR(strstr(buff_in, "password:"), ==, NULL, "read on socket error");

	swrite(sock, (options.pass ? password : PASSWORD), strlen(options.pass ? password : PASSWORD));

	sread(sock, buff_in, 1024);
	ON_ERROR(strstr(buff_in, ">"), ==, NULL, "password: logon failed.");

	PRINTF("%s\n", buff_in);

	swrite_echo(sock, HOME, sizeof(HOME));	/* change to > menu */

	sread(sock, buff_in, 1024);
	ON_ERROR(strstr(buff_in, ">"), ==, NULL, "password: logon failed.");

	PRINTF("%s\n", buff_in);

}

void
logout_procedure()
{
	if (sock != -1) {
		swrite_echo(sock, CLOSECOMM, sizeof(CLOSECOMM));
		sread(sock, buff_in, 1024);
		PRINTF("%s\n", buff_in);
		close(sock);
		sock = -1;
	}
}

/*
 *  commands: ==> add your functions here <===
 */

void
c_raw()
{
	swrite_echo(sock, raw_command, strlen(raw_command));
	sread(sock, buff_in, 1024);
	print_promptless(buff_in);
}

void
c_monitor()
{
	swrite_echo(sock, "", 0);
	for (;;) {
		sread(sock, buff_in, 1024);
		printf("%s\n", buff_in);
	}
}

void
c_getip()
{
	char ip[16];
	char *ptr;
	int res;

	swrite_echo(sock, "ip device", sizeof("ip device"));
	sread(sock, buff_in, 1024);
	PRINTF("%s\n", buff_in);

	ptr = strstr(buff_in, "ppp_device");
	ON_ERROR(ptr, ==, NULL, "ppp_device error.");

	res = sscanf(ptr, "ppp_device %*s %*s %*s %*d %s", ip);
	ON_ERROR(res, !=, 1, "ppp_device error.");

	printf("%s\n", ip);
}

void
c_pnat()
{
        swrite_echo(sock, "nat inbound list", sizeof("nat inbound list"));
        sread(sock, buff_in, 1024);
        print_promptless(buff_in);
}

void
c_version()
{
	swrite_echo(sock, "version", sizeof("version"));
	sread(sock, buff_in, 1024);
	print_promptless(buff_in);
}

void
c_dnsrelay()
{
	swrite_echo(sock, "dnsrelay config", sizeof("dnsrelay config"));
	sread(sock, buff_in, 1024);
	print_promptless(buff_in);
}

void
c_defroute()
{
	swrite_echo(sock, "defaultroute", sizeof("defaultroute"));
	sread(sock, buff_in, 1024);
	print_promptless(buff_in);
}

void
c_uptime()
{
	swrite_echo(sock, "uptime", sizeof("uptime"));
	sread(sock, buff_in, 1024);
	print_promptless(buff_in);
}


void
c_reboot()
{
	swrite_echo(sock, "restart", sizeof("restart"));
	printf("done\n");
	exit(1);
}


/*
 *  commands table: ===> register your functions here <===
 */

item commands[12] = {
	[C_RAW]      = c_raw,
	[C_GETIP]    = c_getip,
	[C_VERSION]  = c_version,
	[C_DNSRELAY] = c_dnsrelay,
	[C_DEFROUTE] = c_defroute,
	[C_UPTIME]   = c_uptime,
	[C_REBOOT]   = c_reboot,
	[C_PNAT]     = c_pnat,
	[C_MONITOR]  = c_monitor
};


/*
 * main
 */

int
main(argc, argv)
	int argc;
	char **argv;
{
	int es;
	while ((es = getopt(argc, argv, "w:p:ijdgurmnvVh")) != EOF)
		switch (es) {
		case 'p':
			password = (char *) strdup(optarg);
			SET(pass);
			break;
		case 'w':
			raw_command = (char *) strdup(optarg);
			comm = C_RAW;
			break;
		case 'i':
			comm = C_GETIP;
			break;
		case 'j':
			comm = C_VERSION;
			break;
		case 'd':
			comm = C_DNSRELAY;
			break;
		case 'g':
			comm = C_DEFROUTE;
			break;
		case 'u':
			comm = C_UPTIME;
			break;
		case 'r':
			comm = C_REBOOT;
			break;
		case 'n':
			comm = C_PNAT;
			break;
		case 'm':
			comm = C_MONITOR;
			break;
		case 'v':
			SET(verbose);
			break;
		case 'V':
			fprintf(stderr, "USR8550 router tool/driver %s\n%s\n", VERSION, AUTHOR);
			exit(1);
		case 'h':
		default:
			usage();
			break;
		}

	argc -= optind;
	argv += optind;

	if (*argv == NULL)
		usage();

	router_host = strdup(*argv);

	if (comm != -1) {
		login_procedure();
		commands[comm].handler();	/* command exectution */
		logout_procedure();
	}
	return 0;
}
