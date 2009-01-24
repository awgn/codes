/*
 * PROXED-NG: Tcp stream redirector/manipulator, conforming C99
 *
 * Copyright (c) 2007 Bonelli Nicola <bonelli@antifork.org>
 * Solaris port Copyright (c) 2003 vjt <vjt@users.sf.net>
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
 */

#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <pthread.h>
#include <poll.h>
#include <assert.h>
#include <sysexits.h>
#include <err.h>

static const char cvsid[]= "$Id: proxed.c,v 1.66 2005/01/21 23:41:11 awgn Exp $";
static const char copyright[] = "Copyright (c) 2004 Bonelli Nicola <bonelli@antifork.org>\n" 
				"Solaris port Copyright (c) 2003 vjt <vjt@users.sf.net>";
/*
 * Macros
 */

#define  VERSION        "3.0"
#define  BUFFLEN       	4906 
#define  MAXQUEUE       10


#ifndef __unused
#define __unused        __attribute__((unused))
#endif

#ifndef __dead
#define __dead          __attribute__((noreturn))
#endif

#define __internal__    "%s:%d %s() :internal",__FILE__,__LINE__,__FUNCTION__

/*
 * typedefs and enums
 */

enum args {
	no_argument = 0,
	required_argument,
};


typedef struct {
	/* host */
	unsigned long listen_host;
	unsigned long connect_dst_host;
	unsigned long connect_src_host;
	unsigned int max_queue;

	/* socket */
	unsigned int af_family;
	unsigned int keepalive;
	unsigned int dontroute;
	unsigned int rcvbuf;
	unsigned int sndbuf;
	unsigned int rcvlowat;
	unsigned int sndlowat;
	unsigned int priority;
	char *bindtodevice;

	/* ip */
	unsigned int ttl;
	unsigned int ttl_assault;
	unsigned int tos;

	/* tcp */
	unsigned int nodelay;
	unsigned int maxseg;
	unsigned int cork;
	unsigned int defer_accept;
	unsigned int keepcnt;
	unsigned int keepidle;
	unsigned int keepintvl;
	unsigned int linger2;
	unsigned int quickack;
	unsigned int syncnt;
	unsigned int window_clamp;
	unsigned int sack_disable;

	/* verbose */
	int verbose;
	int max_client;
} pr_opt_t;


typedef struct {
	int optname;		/* option name */
	int layer;
	void *addr;		/* addr where the value is stored */
	char optchar;		/* char option */
	enum args argument;
	char *text;

} pr_sockopt_t;


typedef struct {
	char *name;
	short port;
	struct sockaddr_storage addr;
} pr_host_t;


typedef struct {
	int insock;
	int outsock;
	int ttl;
	int bytes;
} pr_client_t;


/*
 * Globals
 */

pr_client_t __thread cli;	/* TSD data */

pr_opt_t opt = {
	/* host */
	listen_host:		0,
	connect_dst_host:	0,
	connect_src_host:	0,
	
	/* socket */
	af_family:		AF_INET,
	keepalive:		-1,
	dontroute:		-1,
	rcvbuf:			-1,
	sndbuf:			-1,
	rcvlowat:		-1,
	sndlowat:		-1,
	priority:		-1,
	bindtodevice:		NULL,

	/* ip */
	ttl:			0,
	ttl_assault:		0,
	tos:			-1,

	/* tcp */
	nodelay:		-1,
	maxseg:			-1,
	cork:			-1,
	defer_accept:		-1,
	keepcnt:		-1,
	keepidle:		-1,
	keepintvl:		-1,
	linger2:		-1,
	quickack:		-1,
	syncnt:			-1,
	window_clamp:		-1,
	sack_disable:		-1,

	/* verbose */
	verbose:		0,
	max_client:		MAXQUEUE,
};


pr_sockopt_t sockopt_table[] = {
/* socket */
	{0, 0, 0, 0, 0, " SOCKET\n"},
	{SO_RCVBUF, SOL_SOCKET, &opt.rcvbuf, 'c', required_argument,
	"int                    Set SO_RCVBUF size\n"},
	{SO_SNDBUF, SOL_SOCKET, &opt.sndbuf, 'f', required_argument,
	"int                    Set SO_SNDBUF size\n"},
#ifdef SO_RCVLOWAT
	{SO_RCVLOWAT, SOL_SOCKET, &opt.rcvlowat, 'y', required_argument,
	"int                    Set SO_RCVLOWAT size\n"},
#endif
#ifdef SO_SNDLOWAT
	{SO_SNDLOWAT, SOL_SOCKET, &opt.sndlowat, 'z', required_argument,
	"int                    Set SO_RCVLOWAT size\n"},
#endif
#ifdef SO_PRIORITY
	{SO_PRIORITY, SOL_SOCKET, &opt.priority, 'q', required_argument,
	"int                    Set SO_PRIORITY value\n"},
#endif
#ifdef SO_BINDTODEVICE
	{SO_BINDTODEVICE, SOL_SOCKET, &opt.bindtodevice, 'i', required_argument,
	"device                 Set SO_BINDTODEVICE name\n"},
#endif
	{SO_KEEPALIVE, SOL_SOCKET, &opt.keepalive, 'k', no_argument,
	"                       Set SO_KEEPALIVE\n"},
	{SO_DONTROUTE, SOL_SOCKET, &opt.dontroute, 'r', no_argument,
	"                       Set SO_DONTROUTE\n"},

/* IP */
	{0, 0, 0, 0, 0, " IP\n"},
	{0, 0, 0, 'A', no_argument,
	"                       Set TTL ramping assault\n"},
	{IP_TTL, IPPROTO_IP, &opt.ttl, 'T', required_argument,
	"ttl                    Set IP_TTL\n"},
	{IP_TOS, IPPROTO_IP, &opt.tos, 'O', required_argument,
	"tos                    Set IP_TOS\n"},

/* TCP */
	{0, 0, 0, 0, 0, " TCP\n"},
#ifdef TCP_DEFER_ACCEPT
	{TCP_DEFER_ACCEPT, IPPROTO_TCP, &opt.defer_accept, 'p', required_argument,
	"sec                    Set TCP_DEFER_ACCEPT seconds\n"},
#endif
#ifdef TCP_KEEPIDLE
	{TCP_KEEPIDLE, IPPROTO_TCP, &opt.keepidle, 'l', required_argument,
	"sec                    Set TCP_KEEPIDLE value. sec the connection needs\n"
	"                                 to remain idle before sending keepalive probes\n"},
#endif
#ifdef TCP_KEEPINTVL
	{TCP_KEEPINTVL, IPPROTO_TCP, &opt.keepintvl, 'g', required_argument,
	"sec                    Set TCP_KEEPINTVL sec between individual keepalive\n"
	"                                 probes\n" },
#endif
#ifdef TCP_KEEPCNT
	{TCP_KEEPCNT, IPPROTO_TCP, &opt.keepcnt, 't', required_argument,
	"int                    Set TCP_KEEPCNT value. max number of keepalive\n"
	"                                 probes before dropping the connection\n"},
#endif
#ifdef TCP_SYNCNT
	{TCP_SYNCNT, IPPROTO_TCP, &opt.syncnt, 'u', required_argument,
	"int                    Set TCP_SYNCNT. number of SYN retransmits\n"},
#endif
#ifdef TCP_WINDOW_CLAMP
	{TCP_WINDOW_CLAMP, IPPROTO_TCP, &opt.window_clamp, 'j', required_argument,
	"int                    Set TCP_WINDOW_CLAMP size. min SOCK_MIN_RCVBUF/2\n"},
#endif
	{TCP_MAXSEG, IPPROTO_TCP, &opt.maxseg, 'x', required_argument,
	"int                    Set TCP_MAXSEG. Maximum segment size < MTU\n"},
	{TCP_NODELAY, IPPROTO_TCP, &opt.nodelay, 'n', no_argument,
	"                       Set TCP_NODELAY. Disable Nagle algorithm\n"},
#ifdef TCP_CORK
	{TCP_CORK, IPPROTO_TCP, &opt.cork, 'o', no_argument,
	"                       Set TCP_CORK. If set, don't send partial frames\n"},
#endif
#ifdef TCP_QUICKACK
	{TCP_QUICKACK, IPPROTO_TCP, &opt.quickack, '1', no_argument,
	"                       Set TCP_QUICKACK. (acks are sent immediately)\n"},
#endif
#ifdef TCP_LINGER2
	{TCP_LINGER2, IPPROTO_TCP, &opt.linger2, '2', no_argument,
	"                       Set TCP_LINGER2 lifetime of orphaned FIN_WAIT2\n"
	"                                 state sockets \n"},
#endif
#ifdef TCP_SACK_DISABLE
	{TCP_SACK_DISABLE, IPPROTO_TCP, &opt.sack_disable, 'm', no_argument,
	"                       Set TCP_SACK_DISABLE\n"},
#endif
        {0, 0, 0, '6', no_argument, "                       Use IPv6 addresses\n" },
	{0, 0, 0, 's', required_argument,
	"[host|any][:.]port     Source host and port to listen on\n"},
	{0, 0, 0, 'd', required_argument,
	"host:port              Destination host and port to connect to\n"},
	{0, 0, 0, 'b', required_argument,
	"[host|any][:.]port     Local host and port to bind\n"},

/* common */
	{0, 0, 0, 0, 0, " Common\n"},
	{0, 0, 0, 'V', no_argument,
	"                       Verbose; display verbose messages.\n"},
	{0, 0, 0, 'Q', required_argument,
	"                       N. max of clients; default 10.\n"},
	{0, 0, 0, 'v', no_argument,
	"                       Display version number.\n"},
	{0, 0, 0, 'h', no_argument,
	"                       Print this help\n"},
};

int clients;					/* number of clients */
int true = 1;					/* useful to setsockopt */

pr_host_t listen_host;
pr_host_t local_host;
pr_host_t remote_host;

char *listen_host_string;
char *local_host_string;
char *remote_host_string;

static pthread_mutex_t safe_gethostbyname2_m = PTHREAD_MUTEX_INITIALIZER;

/* lsrr/ssrr : IP options Stevens */

unsigned char optr[40];		/* options frame */
#define LENPTR	&optr[2]
int ocnt;			/* count of addresses */


/*
 * Functions
 */


/* verr()/verrx() wrapper */
__dead void
fatal(const char *pattern,...)
{
        va_list ap;
        va_start(ap, pattern);
        if (errno)
                verr(EX_SOFTWARE, pattern, ap);
        else
                verrx(EX_SOFTWARE, pattern, ap);
}


/*
 * print the usage help on stderr 
 */
__dead void
usage()
{
	extern char *__progname;
	int i;

	fprintf(stderr,"Usage: %s [OPTION]...\n", __progname);

	for (i = 0; i < sizeof(sockopt_table) / sizeof(sockopt_table[0]); i++) {
		if (sockopt_table[i].text == NULL)
			continue;

		fprintf(stderr, (sockopt_table[i].optchar ?  "   -%1$c %2$s" : "%2$s"),
			         sockopt_table[i].optchar, sockopt_table[i].text );
 	
	}
	exit(0);
}


/*
 * inet_ntop wrapper
 */

const char *
generic_inet_ntop(int af, const struct sockaddr *src, char *dst, socklen_t cnt) {
	const char *ret;

	switch(af) {
		case AF_INET: {
			struct sockaddr_in *in = (struct sockaddr_in *)src;
			ret = inet_ntop(af,&in->sin_addr.s_addr, dst, cnt);
			}	
			break;
		case AF_INET6: {
			struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)src;
			ret = inet_ntop(af,&in6->sin6_addr.s6_addr, dst, cnt);
			}
			break;
		default:
			fatal(__internal__);
	}

	return ret;	
}


/*
 * print banner 
 */
void
do_banner()
{

#define banner_int(x)   if (opt.x != -1)   printf("    %s: %d\n",   #x,opt.x) 
#define banner_hex(x)   if (opt.x != -1)   printf("    %s: 0x%x\n", #x,opt.x) 
#define banner_str(x)   if (opt.x != NULL) printf("    %s: %s\n",   #x,opt.x)

	printf("\rProxed-NG %s, %s\n", VERSION, copyright);
	printf("    pid: %d\n", getpid());

	banner_int(verbose);

	printf("SOCKET:\n");
	banner_int(keepalive);
	banner_int(dontroute);
	banner_int(rcvbuf);
	banner_int(sndbuf);
	banner_int(rcvlowat);
	banner_int(sndlowat);
	banner_int(priority);
	banner_str(bindtodevice);

	printf("IP:\n");

	if (opt.ttl) {
		printf( opt.ttl_assault ? 	"    ttl: %d -> {assault}\n" : 
						"    ttl: %d\n",opt.ttl );
	}

	banner_hex(tos);

	printf("TCP:\n");
	banner_int(nodelay);
	banner_int(maxseg);
	banner_int(cork);
	banner_int(defer_accept);
	banner_int(keepcnt);
	banner_int(keepidle);
	banner_int(keepintvl);
	banner_int(linger2);
	banner_int(quickack);
	banner_int(syncnt);
	banner_int(window_clamp);
	banner_int(sack_disable);

	printf(".-  listen : %s\n", listen_host_string );

	if ( local_host_string != NULL)
		printf("|   bind   : %s\n", local_host_string);

	printf("`-> connect: %s\n", remote_host_string);

	return;
}


/*
 * block the signal list of the calling pthread
 */
int
pthread_sigset_block(int n,...)
{
        sigset_t set;
        va_list ap;
        int signum;

        sigemptyset(&set);
        va_start(ap, n);
        for (; n != 0; n--) {
                signum = va_arg(ap, int);
                sigaddset(&set, signum);
        }
	va_end(ap);
        return pthread_sigmask(SIG_BLOCK, &set, NULL);
}


/* 
 * dup_hostent - create hostent in one memory block (KAME/NETBSD) 
 */

struct hostent *dup_hostent(hp)
struct hostent *hp;
{
    struct hostent_block {
        struct hostent host;
        char   *addr_list[1];
    };
        struct hostent_block *hb;
        int     count;
        char   *data;
        char   *addr;

        for (count = 0; hp->h_addr_list[count] != 0; count++)
                /* void */ ;

        if ((hb = (struct hostent_block *) malloc(sizeof(struct hostent_block)
                         + (hp->h_length + sizeof(char *)) * count)) == 0) {
                fprintf(stderr, "Sorry, out of memory\n");
                exit(1);
        }
        memset((char *) &hb->host, 0, sizeof(hb->host));
        hb->host.h_addrtype = hp->h_addrtype;
        hb->host.h_length = hp->h_length;
        hb->host.h_addr_list = hb->addr_list;
        hb->host.h_addr_list[count] = 0;
        data = (char *) (hb->host.h_addr_list + count + 1);

        for (count = 0; (addr = hp->h_addr_list[count]) != 0; count++) {
                hb->host.h_addr_list[count] = data + hp->h_length * count;
                memcpy(hb->host.h_addr_list[count], addr, hp->h_length);
        }
        return &hb->host;
}

void 
del_struct_hostent(struct hostent * dead) {
	int i;
	for (i = 0; dead->h_aliases[i] != NULL; i++)
        	free(dead->h_aliases[i]);
    	for (i = 0; dead->h_addr_list[i] != NULL; i++)
        	free(dead->h_addr_list[i]);
}


/* according to af_family it allocates and returns a sockaddr_in/sockaddr_in6
   structure */ 
int 
getsockaddrbyname(pr_host_t *hp, int af)
{
	struct hostent *h;

	if ( strcmp(hp->name,"any") )
		goto resolve;

	/* any address */

	switch(af) {
	case AF_INET: {
                struct sockaddr_in *in;
                in = (struct sockaddr_in *)&hp->addr;
                in->sin_family = AF_INET;
                in->sin_port   = htons(hp->port);
		in->sin_addr.s_addr = INADDR_ANY;
	} break;

	case AF_INET6: {
                struct sockaddr_in6 *in6;
        	struct in6_addr addr6 = IN6ADDR_ANY_INIT;

        	in6 = (struct sockaddr_in6 *)&hp->addr;
                in6->sin6_family = AF_INET6;
                in6->sin6_port   = htons(hp->port);
                memcpy( &in6->sin6_addr.s6_addr, &addr6, sizeof(struct in6_addr));
	} break;

	default:
		fatal(__internal__);
	}		

        return 0;

  resolve:

        pthread_mutex_lock(&safe_gethostbyname2_m); /* <--------- atomic section --------- */
	
	h = gethostbyname2(hp->name,af);
	if(h == NULL)
		fatal("gethostbyname(\"%s\")",hp->name);

	switch (af) {
	case AF_INET: { 
		struct sockaddr_in *in;
		in = (struct sockaddr_in *)&hp->addr;
		in->sin_family = AF_INET;
		in->sin_port   = htons(hp->port);
		memcpy( &in->sin_addr.s_addr, h->h_addr, h->h_length);
		}
		break;
	case AF_INET6: { 
		struct sockaddr_in6 *in6;
		in6 = (struct sockaddr_in6 *)&hp->addr;
                in6->sin6_family = AF_INET6;
                in6->sin6_port   = htons(hp->port);
                memcpy( &in6->sin6_addr.s6_addr, h->h_addr, h->h_length);
		}
		break;
	default:
		fatal(__internal__);
	}

	pthread_mutex_unlock(&safe_gethostbyname2_m); /* <--------- EOS --------- */
	return 0;
}


/*
 * given hostname_port it returns the parsed pr_host_t type
 */
int
parse_host_port(pr_host_t *host, const char *str)
{
        char *s, *p;
	char den;

	if (str == NULL)
		fatal(__internal__);

	den = ( opt.af_family == AF_INET ? ':' : '.' );
	s = strdup (str);
	p = strrchr (s, den);
	if (p == NULL)
		fatal("[%s] host/port parser error",str);

        *p++ = '\0';

	host->name = s;
        host->port = atoi(p);
	return 0;
}


/*
 * kill the thread closing the sockets
 */ 
void
client_exit()
{
	clients--;

	close(cli.insock);
	close(cli.outsock);

	pthread_exit(NULL);
}


/*
 * OpenBSD atomcio. Copyright (c) 1999 Theo de Raadt
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
			client_exit();
		case 0:
			return res;
		default:
			pos += res;
		}
	}

	return pos;
}


/*
 * send a message to the socket
 */
int
sendsock_msg(int sd, char *pattern,...)
{
	char *buff;
	va_list ap;
	int len, ret;

	va_start(ap, pattern);
	len = vasprintf(&buff, pattern, ap);
	va_end(ap);

	if ((ret = atomicio(write, sd, buff, len)) < 1)
		client_exit();
	free(buff);
	return ret;
}


int 
setsockopt_v(int in, int s,int level,int optname,const void *optval,socklen_t optlen) { 
	int r;
	if ( (r=setsockopt(s,level,optname,optval,optlen)) == -1 ) 
        	sendsock_msg(in, "debug: err: setsockopt(%d,%d,%d,%d ... )=-1 (%s)\n",s,\
        	level,optname,optval,strerror(errno)); 
	return r;
} 

/*
 * create a socket seting up the relating options.
 */ 
int
do_create_socket(int in, int ttl)
{
	struct linger li;
	int i,s;

	/* To avoid TIME_WAIT state */
	li.l_onoff = 1;
	li.l_linger = 0;

	/* create a socket */
	if ((s = socket(opt.af_family, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		if (opt.verbose)
			sendsock_msg(in, "debug: err: cannot create socket\n");
		return -1;
	}

	/* socket options: SOL_SOCKET: reuse addr | linger {1,0}: fast rst. */
	setsockopt_v(in, s, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true));
	setsockopt_v(in, s, SOL_SOCKET, SO_LINGER, (void *) &li, sizeof(li));

	for (i = 0; i < sizeof(sockopt_table) / sizeof(sockopt_table[0]); i++) {

		if (         sockopt_table[i].addr == NULL ||
		    *(int *) sockopt_table[i].addr == -1   ||
	            *(int *) sockopt_table[i].addr == 0	)
			continue;

		if (sockopt_table[i].addr == &opt.ttl) {
			/* ttl */
			setsockopt_v(in, s, sockopt_table[i].layer, sockopt_table[i].optname, (void *) &ttl, sizeof(int));
			continue;
		}
#ifdef SO_BINDTODEVICE
		if (sockopt_table[i].addr == &opt.bindtodevice) {
			/* SO_BINDTODEVICE */
			if (opt.bindtodevice == NULL)
				continue;
			setsockopt_v(in, s, SOL_SOCKET, SO_BINDTODEVICE, opt.bindtodevice, 
				   strlen(opt.bindtodevice));
			continue;
		}
#endif
		/* otherwise: generic {socket,ip,tcp} option */

		setsockopt_v(in, s, sockopt_table[i].layer, sockopt_table[i].optname, 
			   sockopt_table[i].addr, sizeof(int));
	}
	return s;
}


/*
 * bind socket
 */
int
do_bind_socket(int in, int s)
{
	int r;
	/* temporary setup remote_addr sockaddr structure for local binding */

	if (local_host_string == NULL)
		return -1;

	/* bind */
	if ((r=bind(s, (struct sockaddr *)&local_host.addr, 
	opt.af_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6))) == -1) {
		if (opt.verbose) {
                	char tmp[40];
                	generic_inet_ntop(opt.af_family, (struct sockaddr *)&local_host.addr, tmp, sizeof(tmp));
			sendsock_msg(in, "error: cannot bind src_host=%s, src_port=%d (%s)\n",
				tmp, local_host.port, strerror(errno));
		}
	}

	return r;
}


/*
 * connect socket 
 */
int
do_connect()
{
        int distance, time_exceed;
	int ret;

	distance=-1;	/* unknown path */
	
        do {
		time_exceed=0;

                ret = connect(cli.outsock, (struct sockaddr *)&remote_host.addr, 
	        opt.af_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6));

                if (ret == 0 || !opt.ttl_assault)
			return ret;	/* return from do_connect() */

                switch (errno) {
                case EINTR:
                case ENETUNREACH:
                case EHOSTUNREACH:
                case ETIMEDOUT:
                        time_exceed = 1;
                        break;
                case ECONNREFUSED:
			if ( distance == -1)
				distance = cli.ttl;
                        break;
		default:
			fatal("!errno=%d! unknown connect() error!",errno);
                }

		if (opt.verbose)
		sendsock_msg(cli.insock, ( distance != -1 ?
                                     "ttl: (%1$d) -> (%2$d)\n" :
                                     "ttl: (%1$d) -> (?)\n" ),
                                      cli.ttl, cli.ttl - distance);
	
		/* create a new socket */
                close(cli.outsock);
                cli.outsock = do_create_socket(cli.insock, ++cli.ttl);
		if (cli.outsock == -1)
			client_exit();

		/* socket is bound then */
                do_bind_socket(cli.insock, cli.outsock);
        }
        while (ret == -1);

	return ret;
}


/*
 * main client handler
 */
void *
client_handler(void *s)
{
	char buff[BUFFLEN];
	struct pollfd pfd[2];
	int ret;

	cli.ttl     = opt.ttl;
	cli.insock  = (int) s;	
	cli.outsock = 0;
	cli.bytes   = 0;
	
	pthread_sigset_block(4,SIGHUP,SIGSEGV,SIGPIPE,SIGURG);

	/* create a socket */
	cli.outsock = do_create_socket(cli.insock, opt.ttl);
       	if (cli.outsock == -1) 
		client_exit();
 
	if (opt.verbose) {
		sendsock_msg(cli.insock, "debug: Proxed-NG %s, %s\n", VERSION, copyright);
		sendsock_msg(cli.insock, "debug: Socket (%d) accepted\n", cli.insock);
		sendsock_msg(cli.insock, "debug: Pthread descriptor (%x); clients=%d\n", pthread_self(), clients);
                sendsock_msg(cli.insock, "debug: New socket (%d) created\n", cli.outsock);
	}

	/* bind socket */
	do_bind_socket(cli.insock, cli.outsock);

	/* connect */
	if (opt.verbose) {
		char tmp[40];
		generic_inet_ntop(opt.af_family, (struct sockaddr *)&remote_host.addr, tmp, sizeof(tmp));
		sendsock_msg(cli.insock, "debug: Connecting to %s_%d\n", tmp, remote_host.port);
	}

	if ( do_connect() == -1) {
		sendsock_msg(cli.insock, "debug: server-error: connect() %s\n", strerror(errno));
		client_exit();
	}

	/* established message */
	if (opt.verbose)
		sendsock_msg(cli.insock, "debug: State ESTABLISHED\n");

	/* I/O read/write */
	pfd[0].fd = cli.insock;
	pfd[1].fd = cli.outsock;
	pfd[0].events = POLLIN | POLLRDNORM | POLLERR;
	pfd[1].events = POLLIN | POLLRDNORM | POLLERR;

	memset(buff, 0, BUFFLEN);

	for (;;) {
		pfd[0].revents = 0;
		pfd[1].revents = 0;
		ret = poll(pfd, 2, -1);

		if (ret == 0)
			continue;

		if (ret == -1) { 
			switch (errno) {
			case EINTR:
				continue;
			default:
				goto c_exit;
			}
		}

		if (pfd[0].revents & (POLLRDNORM | POLLIN | POLLERR)) {
			cli.bytes = read(cli.insock, buff, BUFFLEN);
			if (cli.bytes < 1)	/* 0: connection closed by peer, -1:
					 * error or connection reset by peer */
				goto c_exit;
			atomicio(write, cli.outsock, buff, cli.bytes);
		}
		if (pfd[1].revents & (POLLRDNORM | POLLIN | POLLERR)) {
			cli.bytes = read(cli.outsock, buff, BUFFLEN);
			if (cli.bytes < 1)	/* 0: connection closed by peer, -1:
					 * error or connection reset by peer */
				goto c_exit;
			atomicio(write, cli.insock, buff, cli.bytes);
		}
	}

c_exit:			/* client exit */
	client_exit();
	return NULL;
}


/* 
 * initialize server
 */
int
server_init()
{
	int s;

	/* create af_inet socket */
	if ((s = socket(opt.af_family, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		fatal("socket");
	}

	/* socket options: reuseaddr */
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &true, sizeof(true));

	/* setup listen_host */
	parse_host_port(&listen_host, listen_host_string);
	getsockaddrbyname(&listen_host, opt.af_family);

        /* bind */
        if (bind(s, (struct sockaddr *)&listen_host.addr,
                     opt.af_family == AF_INET ? sizeof(struct sockaddr_in) :
                                                sizeof(struct sockaddr_in6)) == -1)
                fatal("bind");

        /* listen */
        if (listen(s, opt.max_client ? : MAXQUEUE) == -1)
                fatal("listen");

	/* setup local_host */
	if (local_host_string != NULL) {
        	parse_host_port(&local_host, local_host_string);
		getsockaddrbyname(&local_host, opt.af_family);
	}

	/* setup remote_host */
	parse_host_port(&remote_host, remote_host_string);
	getsockaddrbyname(&remote_host, opt.af_family);

	/* print banner */
	do_banner();

	/* ready to accept incoming connections */
	return s;
}


/* 
 * it accepts new clients and safe creates the handlers
 */
void
server_core(int sock)
{
	struct sockaddr_storage peer;
	socklen_t addrlen;
	pthread_t void_pd;
	int s;

	addrlen = sizeof(struct sockaddr_storage);

	for (;;) {
		if ((s = accept(sock, (struct sockaddr *)&peer, &addrlen)) != -1) {
			clients++;

			pthread_create(&void_pd, NULL, client_handler, (void *)s);
			pthread_detach(void_pd);
		}
	}

	return;	 /* unreachable */
}


/*                       *   
 *   P R O X E D - N G   *
 *                       */

int
main(int argc, char *argv[])
{
	char opt_string[80];
	int i, j, es;

	/* create the opt string */
	for (i = 0, j = 0; i < sizeof(sockopt_table) / sizeof(sockopt_table[0]); i++) {
		if (sockopt_table[i].optchar == 0)
			continue;

		/* adding a new option */
		opt_string[j++] = sockopt_table[i].optchar;

		if (sockopt_table[i].argument == no_argument)
			continue;

		/* options with argument */
		opt_string[j++] = ':';

	}
	opt_string[j++] = '\0';

	while ((es = getopt(argc, argv, opt_string)) != EOF)
		switch (es) {
		case '6':
			opt.af_family = AF_INET6;
			break;
		case 's':
			listen_host_string = optarg;
			break;
		case 'd':
			remote_host_string = optarg;
			break;
		case 'b':
			local_host_string = optarg;
			break;
		case 'T':
			opt.ttl = strtol(optarg, (char **) NULL, 0);
			break;
		case 'O':
			opt.tos = strtol(optarg, (char **) NULL, 0);
			break;
		case 'v':
			fprintf(stderr, "Proxed-NG %s, %s\n", VERSION, copyright);
			exit(1);
			break;
		case 'c':
			opt.rcvbuf = strtol(optarg, (char **) NULL, 0);
			break;
		case 'f':
			opt.sndbuf = strtol(optarg, (char **) NULL, 0);
			break;
		case 'x':
			opt.maxseg = strtol(optarg, (char **) NULL, 0);
			break;
		case 'y':
			opt.rcvlowat = strtol(optarg, (char **) NULL, 0);
			break;
		case 'z':
			opt.sndlowat = strtol(optarg, (char **) NULL, 0);
			break;
		case 'q':
			opt.priority = strtol(optarg, (char **) NULL, 0);
			break;
		case 'i':
			opt.bindtodevice = optarg;
			break;
		case 'p':
			opt.defer_accept = strtol(optarg, (char **) NULL, 0);
			break;
		case 'l':
			opt.keepidle = strtol(optarg, (char **) NULL, 0);
			break;
		case 'g':
			opt.keepintvl = strtol(optarg, (char **) NULL, 0);
			break;
		case 't':
			opt.keepcnt = strtol(optarg, (char **) NULL, 0);
			break;
		case 'u':
			opt.syncnt = strtol(optarg, (char **) NULL, 0);
			break;
		case 'j':
			opt.window_clamp = strtol(optarg, (char **) NULL, 0);
			break;
		case 'Q':
			opt.max_client = strtol(optarg, (char **) NULL, 0);
			break;
		case 'A':
			opt.ttl_assault = 1;
			break;
		case 'k':
			opt.keepalive = 1;
			break;
		case 'r':
			opt.dontroute = 1;
			break;
		case 'n':
			opt.nodelay = 1;
			break;
		case 'V':
			opt.verbose = 1;
			break;
		case 'o':
			opt.cork = 1;
			break;
		case '1':
			opt.quickack = 1;
			break;
		case '2':
			opt.linger2 = 1;
			break;
		case 'm':
			opt.sack_disable = 1;
			break;
		case 'h':
			usage();
		default:
			exit(-1);
			break;
		}

	argc -= optind;
	argv += optind;

	/* required options */
	if (listen_host_string == NULL)
		fatal("listen host/port required");

	if (remote_host_string == NULL)
		fatal("destination host/port required");	
	
	if (!opt.ttl && opt.ttl_assault)
		opt.ttl = 1;/* default setting if ttl assault is set */

	/* signal */

	signal(SIGHUP,  SIG_IGN);
	signal(SIGSEGV, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);	/* write/send to a broken pipe terminates the process */
	signal(SIGURG,  SIG_IGN);	/* dont care about oob message */

	switch (fork()) {
	case -1:
		fprintf(stderr, "fork() err:\n");
		exit(1);
	case 0: {
		sleep(1);
		setsid();
	
		int s;
		s = server_init();
		server_core(s);

	} break;
	default:
		fprintf(stdout, "binding socket...\n");
		exit(0);
	}

	return 0;
}

