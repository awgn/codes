/*
 * lindump: high performance IP sniffer/dumper ... for linux 2.4.
 *
 * gcc -O3 -march=pentiumN <-DFRAME_FILTER> <-DCACHE=XX> -o lindump.
 *
 *         -DFRAME_FILTER 	add frame filters
 *         -DCACHE=XX		add XX Mbyte cache
 *
 * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
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

#define _GNU_SOURCE

#ifndef FRAME_FILTER
#warning "-DFRAME_FILTER add frame filter capability"
#endif
#ifndef CACHE
#warning "-DCACHE=XX add XX Mbyte cache for better performance"
#endif

#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <net/if.h>
#include <features.h>		/* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>	/* the L2 protocols */
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>	/* The L2 protocols */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#define CONFIG_X86_CMPXCHG
#include <asm/system.h>

static char cvsid[] = "$Id: lindump.c,v 1.54 2003/06/25 21:49:05 awgn Exp $";

/* Macro */

#define SOCKET(x,y,z)   socket(x,y,z); fprintf(stderr,"[+] create: socket(%s,%s,%s)\n",#x,#y,#z)

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#define U(x)     ( (x) > 0 ? x  : 0 )
#define ABS(x)   ( (x) < 0 ? -(x) : (x) )
#define bit         >>3
#define kbit        <<7
#define mbit        <<17
#define gbit        <<27
#define kbyte       <<10
#define mbyte       <<20
#define gbyte       <<30

#define BUFFLEN		ETH_HLEN+sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(struct udp_data)

#define OPT_DUMP_MIN 		1	/* tstamp framelen */
#define OPT_DUMP_IPID		2	/* tstamp framelen ipid */
#define OPT_DUMP_RUDE		3	/* tstamp framelen flow_id
					 * sequence_number <-- for rude
					 * generator */
#define OPT_DUMP_RAW		4
#define OPT_DUMP_REPLAY		5
#define OPT_DUMP_MACSTAT	6
#define OPT_DUMP_RING		7


#define OPT_SOCK_RAW		1
#define OPT_SOCK_DGRAM		2

#define OPT_PROM        (1U<<0)
#define OPT_POLL	(1U<<1)

#define MAX_FILTER		7
#define OPT_FILTER_HOST		(1<<PACKET_HOST)
#define OPT_FILTER_BROADCAST	(1<<PACKET_BROADCAST)
#define OPT_FILTER_MULTICAST    (1<<PACKET_MULTICAST)
#define OPT_FILTER_OTHERHOST	(1<<PACKET_OTHERHOST)
#define OPT_FILTER_OUTGOING	(1<<PACKET_OUTGOING)
#define OPT_FILTER_LOOPBACK     (1<<PACKET_LOOPBACK)
#define OPT_FILTER_FASTROUTE    (1<<PACKET_FASTROUTE)

#define TP_BLOCK_SIZE           4096
#define TP_FRAME_SIZE           256
#define TP_BLOCK_NR             2048
#define TP_FRAME_NR             TP_BLOCK_NR*(TP_BLOCK_SIZE/TP_FRAME_SIZE)
#define TP_FRAME_NR_MASK        TP_FRAME_NR-1


/* kernel hack */

#ifndef PACKET_MACSTAT
#define PACKET_MACSTAT		8
/* apply patch to net/packet/af_packet.c */
struct tpacket_macstat {
	struct timeval tstamp;
	unsigned short len;
	unsigned short ip_id;
	unsigned char  tos;
	unsigned char  pkttype;
} __attribute__((packed));

#endif

/* rude udp data structure */
struct udp_data {
	unsigned long sequence_number;
	unsigned long tx_time_seconds;
	unsigned long tx_time_useconds;
	unsigned long flow_id;
	unsigned long dest_addr;
}        __attribute__((packed));


/* kernel frame structure */
struct frame_map {
	struct tpacket_hdr tp_h __attribute__((aligned(TPACKET_ALIGNMENT)));
	struct sockaddr_ll s_ll __attribute__((aligned(TPACKET_ALIGNMENT)));
};

/***/

#ifdef CACHE
char *big_buffer;
char *big_limit;
#endif
char buffer[BUFFLEN];

struct iphdr *iph;
struct udphdr *udph;
struct udp_data *datah;
struct sched_param sched;

extern char *__progname;

char *output = "stdout";	/* filename */
char *input;
char *dev_name;
char frame_t[] = {
	'<',			/* incoming */
	'B',			/* broadcast */
	'M',			/* multicast */
	'O',			/* promisc */
	'>',			/* outgoing */
	'L',			/* loopback */
	'F',			/* fastroute */
};


int f_filter;
int so_packet;
int options;
int opt_dump;
int opt_socket;
int dev_id;
int n_cap;
int tim;
int eth_p;
int priority = 99;		/* highest priority by default */

int (*frame_filter) (int);



/* rx_ring */
struct tpacket_req tp_req;
char *arena;			/* map the shared memory for packets */
struct iovec *ring;		/* iovec points to frames into arena */

FILE *fileout;
FILE *filein;

int fdout;			/* raw dump */

static void fatal(char *pattern,...) __attribute__((noreturn));
static void
fatal(char *pattern,...)
{
	va_list ap;
	char *mg;

	va_start(ap, pattern);
	vasprintf(&mg, pattern, ap);
	va_end(ap);

	if (errno)
		perror(mg);
	else
		fputs(mg, stderr), fputc('\n', stderr);

	abort();
}

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

void
set_nonblock(sock)
	int sock;
{
	register int val = 0;
	val = fcntl(sock, F_GETFL, val);
	val |= O_NONBLOCK;
	fcntl(sock, F_SETFL, val);
	return;
}


void
set_block(sock)
	int sock;
{
	register int val = 0;
	val = fcntl(sock, F_GETFL, val);
	val &= ~O_NONBLOCK;
	fcntl(sock, F_SETFL, val);
	return;
}


/* --- dump --- */

#define DUMP_MSG(S,b)	do {							\
        if (options & OPT_POLL)							\
                fprintf(stderr,"[P] %s(%d): %s\n",__FUNCTION__,(b),(S));	\
        else									\
                fprintf(stderr,"[+] %s(%d): %s\n",__FUNCTION__,(b),(S));	\
} while (0)


void
__dump(register int byte)
{
	struct sockaddr_ll m;
	struct timeval tstamp;
	socklen_t len;

	len = sizeof(struct sockaddr_ll);
	DUMP_MSG("tstamp, framelen", byte);

	for (;;) {
		if (recvfrom(so_packet, buffer, byte, 0, (struct sockaddr *) & m, &len) == -1)
			continue;
#ifdef FRAME_FILTER
		if (frame_filter(m.sll_pkttype))
			continue;
#endif
		ioctl(so_packet, SIOCGSTAMP, &tstamp);
		n_cap++;

		fprintf(fileout, "%c:%u.%.6u %u\n",
			frame_t[m.sll_pkttype], (unsigned int) tstamp.tv_sec,
			(unsigned int) tstamp.tv_usec, ntohs(iph->tot_len) + ETH_HLEN);
	}
}


void
__dump_ipid(register int byte)
{
	struct sockaddr_ll m;
	struct timeval tstamp;
	socklen_t len;

	len = sizeof(struct sockaddr_ll);
	DUMP_MSG("tstamp, framelen, ip_id, tos", byte);

	for (;;) {
		if (recvfrom(so_packet, buffer, byte, 0, (struct sockaddr *) & m, &len) == -1)
			continue;
#ifdef FRAME_FILTER
		if (frame_filter(m.sll_pkttype))
			continue;
#endif
		ioctl(so_packet, SIOCGSTAMP, &tstamp);
		n_cap++;
		fprintf(fileout, "%c:%u.%.6u %u %u 0x%.2x\n",
			frame_t[m.sll_pkttype], (unsigned int) tstamp.tv_sec, (unsigned int) tstamp.tv_usec,
			ntohs(iph->tot_len) + ETH_HLEN, ntohs(iph->id),iph->tos);
	}
}

/* raw dump */

void
__dump_raw(register int byte)
{
        struct sockaddr_ll m;
        struct iovec io_vector[5] __attribute__((unused));
        struct timeval tstamp;
        socklen_t le;

#ifdef CACHE
        register short *buff=(short *)big_buffer;
	register char  *buff_c;
#endif

        le = sizeof(struct sockaddr_ll);

        DUMP_MSG("tstamp, framelen, ip_id, tos", byte);
	write(fdout, &opt_dump, sizeof(long));

#ifndef CACHE
        io_vector[0].iov_base = &tstamp;
        io_vector[0].iov_len = sizeof(struct timeval);
        io_vector[1].iov_base = &iph->tot_len;
        io_vector[1].iov_len = sizeof(short);
        io_vector[2].iov_base = &iph->id;
        io_vector[2].iov_len = sizeof(short);
	io_vector[3].iov_base = &iph->tos;
	io_vector[3].iov_len = sizeof(char);
        io_vector[4].iov_base = &m.sll_pkttype;
        io_vector[4].iov_len = sizeof(char);   
#endif

#ifdef CACHE
        for (; (void *)buff < (void *)big_limit;) {
#else
        for (;;) {
#endif
                if (recvfrom(so_packet, buffer, byte, 0, (struct sockaddr *) &m, &le) == -1)
                        continue;
#ifdef FRAME_FILTER
                if (frame_filter(m.sll_pkttype))
                        continue;
#endif
                ioctl(so_packet, SIOCGSTAMP, &tstamp);
                n_cap++;
#ifdef CACHE
		/* memcpy */
                __builtin_memcpy(buff,&tstamp,sizeof(struct timeval));
                buff+=4;
                *buff++ = iph->tot_len;
                *buff++ = iph->id;

		buff_c = (char *)buff++;
		*buff_c++ = iph->tos;
		*buff_c++ = m.sll_pkttype; 
#else
		/* writev */
                writev(fdout, io_vector, 5);
#endif
        }
}


void
__dump_ring(int byte)
{
	struct frame_map *frame;
	struct iovec io_vector[5] __attribute__((unused));
	struct pollfd pl;
	int iph_pad;
	register int i = 0;
#ifdef CACHE
	register short *buff=(short *)big_buffer;
	register char  *buff_c;
#endif

	DUMP_MSG("tstamp, framelen, ip_id, tos", byte);
	write(fdout, &opt_dump, sizeof(long));

	iph_pad = sizeof(struct frame_map) + sizeof(short) + ETH_HLEN;

        pl.fd = so_packet;
        pl.events = POLLIN | POLLERR;
        pl.revents = 0;

	frame = (struct frame_map *) ring[0].iov_base;
	iph = (struct iphdr *) ((void *) ring[0].iov_base + iph_pad);

#ifdef CACHE
        for (; (void *)buff < (void *)big_limit;) {
#else
        for (;;) {
#endif
		while (frame->tp_h.tp_status) {
#ifdef FRAME_FILTER
			if (!frame_filter(frame->s_ll.sll_pkttype)) {
#endif
				n_cap++;
#ifdef CACHE
				/* memcpy */
				__builtin_memcpy(buff,&(frame->tp_h.tp_sec),sizeof(struct timeval));
				buff+=4;
				*buff++ = iph->tot_len;
				*buff++ = iph->id;

				buff_c = (char *)buff++;
				*buff_c++ = iph->tos;
				*buff_c++ = frame->s_ll.sll_pkttype;
#else
				/* writev */
                        	io_vector[0].iov_base = &(frame->tp_h.tp_sec);
                        	io_vector[0].iov_len = sizeof(struct timeval);
                        	io_vector[1].iov_base = &iph->tot_len;
                        	io_vector[1].iov_len = sizeof(short);
                        	io_vector[2].iov_base = &iph->id;
                        	io_vector[2].iov_len = sizeof(short);
                        	io_vector[3].iov_base = &iph->tos;
                        	io_vector[3].iov_len = sizeof(char);   
                                io_vector[4].iov_base = &(frame->s_ll.sll_pkttype);
                                io_vector[4].iov_len = sizeof(char);  
				writev(fdout, io_vector, 5);
#endif

#ifdef FRAME_FILTER
			}
#endif
			frame->tp_h.tp_status = 0;
			mb();
			i++;
			i &= TP_FRAME_NR_MASK;
			//%TP_FRAME_NR
			frame = (struct frame_map *) ring[i].iov_base;
			iph = (struct iphdr *) ((void *) ring[i].iov_base + iph_pad);
		}
		if (poll(&pl, 1, -1) == -1)
			fatal("poll");
	}

}


void
__dump_macstat(register int byte)
{
	struct tpacket_macstat *tp;
	struct sockaddr_ll m;
	register char *buff;
	socklen_t le;

        DUMP_MSG("tstamp, framelen, ip_id, tos", byte);
        write(fdout, &opt_dump, sizeof(long));

#ifdef CACHE
        buff = big_buffer;
	tp = (struct tpacket_macstat *) big_buffer;
#else
        buff = buffer;
	tp = (struct tpacket_macstat *) buffer;
#endif
	le = sizeof(struct sockaddr_ll);

#ifdef CACHE
	for (; buff < big_limit;) {
#else
	for (;;) {
#endif
		if (recvfrom(so_packet, buff, byte, 0, (struct sockaddr *) & m, &le) == -1)
			continue;
#ifdef FRAME_FILTER
		if (frame_filter(m.sll_pkttype))
			continue;
#endif
		n_cap++;
	//	*(&tp->tos+1) = m.sll_pkttype;	/* overwrite the last byte of MAC header with sll_pkttype */
#ifndef CACHE
		write(fdout, buffer, sizeof(struct tpacket_macstat));
	}
#else
		buff += sizeof(struct tpacket_macstat); 
	}
	fprintf(stderr, "[*] %d Mbyte cache full!\n", CACHE);
	exit(0);
#endif
}


/* dump rude flows */
void
__dump_rude(register int byte)
{
        struct sockaddr_ll m;
        struct timeval tstamp;
        socklen_t len;
        int rcv;

        len = sizeof(struct sockaddr_ll);

        DUMP_MSG("tstamp, framelen, flow_id, seq_number", byte);

        for (;;) {
                rcv = recvfrom(so_packet, buffer, byte, 0, (struct sockaddr *) & m, &len);
#ifdef FRAME_FILTER
                if (frame_filter(m.sll_pkttype))
                        continue;
#endif
                ioctl(so_packet, SIOCGSTAMP, &tstamp);
                n_cap++;

                if (rcv < byte || iph->protocol != IPPROTO_UDP)
                        continue;

                fprintf(fileout, "R:%u.%.6u %u %u %u\n",
                (unsigned int) tstamp.tv_sec, (unsigned int) tstamp.tv_usec,
                        ntohs(iph->tot_len) + ETH_HLEN, ntohl(datah->flow_id), ntohl(datah->sequence_number));
        }
}


/* replay raw files */
void
__dump_read()
{
	struct tpacket_macstat raw;
	int type;

	if (fread(&type, sizeof(long), 1, filein) != 1)
		fatal("%s(): file empty or truncated", __FUNCTION__);

	switch (type) {
	case OPT_DUMP_RAW:
		fprintf(stderr, "[*] file %s: DUMP_RAW format\n", input);
		goto next;
	case OPT_DUMP_RING:
		fprintf(stderr, "[*] file %s: DUMP_RING format\n", input);
next:		for (; feof(filein) == 0;) {
			fread(&raw, sizeof(struct tpacket_macstat), 1, filein);
			fprintf(fileout, "%c:%u.%.6u %u %u 0x%.2x\n", frame_t[raw.pkttype], (u_int) raw.tstamp.tv_sec,
			(u_int) raw.tstamp.tv_usec, ntohs(raw.len) + ETH_HLEN, ntohs(raw.ip_id),raw.tos);
			n_cap++;
		}
		break;
	case OPT_DUMP_MACSTAT:
		fprintf(stderr, "[*] file %s: DUMP_MACSTAT format\n", input);
		for (; feof(filein) == 0;) {
			fread(&raw, sizeof(struct tpacket_macstat), 1, filein);
			fprintf(fileout, "%c:%u.%.6u %u %u 0x%.2x\n", frame_t[raw.pkttype], (u_int) raw.tstamp.tv_sec,
			(u_int) raw.tstamp.tv_usec, raw.len, ntohs(raw.ip_id),raw.tos);
			n_cap++;
		}
		break;
	default:
		fatal("%s(): bad file format", __FUNCTION__);
	}
}


int datalen[4][8];
void
setup_datalen()
{
	datalen[OPT_SOCK_RAW][OPT_DUMP_MIN] = ETH_HLEN + sizeof(struct iphdr);
	datalen[OPT_SOCK_DGRAM][OPT_DUMP_MIN] = sizeof(struct iphdr);
	datalen[OPT_SOCK_RAW][OPT_DUMP_IPID] = ETH_HLEN + sizeof(struct iphdr);
	datalen[OPT_SOCK_DGRAM][OPT_DUMP_IPID] = sizeof(struct iphdr);
	datalen[OPT_SOCK_RAW][OPT_DUMP_RUDE] = ETH_HLEN +sizeof(struct iphdr)+sizeof(struct udphdr)+sizeof(struct udp_data);
	datalen[OPT_SOCK_DGRAM][OPT_DUMP_RUDE] = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(struct udp_data);
	datalen[OPT_SOCK_RAW][OPT_DUMP_RAW] = ETH_HLEN + sizeof(struct iphdr);
	datalen[OPT_SOCK_DGRAM][OPT_DUMP_RAW] = sizeof(struct iphdr);
	datalen[OPT_SOCK_RAW][OPT_DUMP_MACSTAT] = ETH_HLEN;
	datalen[OPT_SOCK_DGRAM][OPT_DUMP_MACSTAT] = 0;
}


void
dumper()
{
	setup_datalen();
	switch (opt_dump) {
	case OPT_DUMP_MIN:
		__dump(datalen[opt_socket][opt_dump]);
		break;
	case OPT_DUMP_IPID:
		__dump_ipid(datalen[opt_socket][opt_dump]);
		break;
	case OPT_DUMP_RUDE:
		__dump_rude(datalen[opt_socket][opt_dump]);
		break;
	case OPT_DUMP_RAW:
		__dump_raw(datalen[opt_socket][opt_dump]);
		break;
	case OPT_DUMP_REPLAY:
		__dump_read();
		break;
	case OPT_DUMP_RING:
		__dump_ring(datalen[opt_socket][opt_dump]);
		break;
	case OPT_DUMP_MACSTAT:
		__dump_macstat(datalen[opt_socket][opt_dump]);
		break;
	default:
		fatal("unknown opt_socket/opt_dump mode");
	}
}


/*
 * get the interface id from name
 */
int
iface_getid(int fd, const char *dev)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

	if (ioctl(fd, SIOCGIFINDEX, &ifr) == -1)
		fatal("interface %s not found", dev);

	return ifr.ifr_ifindex;
}


/*
 * set promiscus mode for interface
 */
void
iface_setprom(int sock_fd, char *dev)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) == -1)
		fatal("ioctl: SIOCGIFFLAGS");

	ifr.ifr_flags |= IFF_PROMISC;

	if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) == -1)
		fatal("ioctl: SIOCSIFFLAGS");

	fprintf(stderr, "[*] device: %s enter in promiscuous mode\n", dev);
}


/*
 * remove promisc for interface
 */
void
iface_extprom(int sock_fd, char *dev)
{
	struct ifreq ifr;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

	if (ioctl(sock_fd, SIOCGIFFLAGS, &ifr) == -1)
		fatal("ioctl: SIOCGIFFLAGS");

	ifr.ifr_flags &= ~IFF_PROMISC;

	if (ioctl(sock_fd, SIOCSIFFLAGS, &ifr) == -1)
		fatal("ioctl: SIOCSIFFLAGS");

	fprintf(stderr, "[*] device: %s exit from promiscuous mode\n", dev);

}


/*
 * SIGINT handler
 */

void
exit_handler(int i)
{
	exit(i);
}

void destructor() __attribute__((noreturn, destructor));
void
destructor(int i)
{
	struct tpacket_stats kstats;
	size_t len = sizeof(struct tpacket_stats);

#ifdef CACHE
	if (	opt_dump == OPT_DUMP_RAW     ||
		opt_dump == OPT_DUMP_MACSTAT ||
		opt_dump == OPT_DUMP_RING 	) {
		atomicio(write, fdout, big_buffer, sizeof(struct tpacket_macstat) * n_cap);
		fprintf(stderr, "[+] cache flushed\n");
	}
#endif
	if (strcmp(output, "stdout")) {
		fprintf(stderr, "[-] closing(%s)\n", output);
		close(fdout);
		fclose(fileout);
	}
	if (filein != NULL)
		fclose(filein);

	if (dev_name != NULL && (options & OPT_PROM)) {
		iface_extprom(so_packet, dev_name);
	}
	if (so_packet && getsockopt(so_packet, SOL_PACKET, PACKET_STATISTICS, &kstats, &len) > -1) {
		fprintf(stderr, "[*] %d packets received, %d packets dropped, %d packets captured\n",
			kstats.tp_packets, kstats.tp_drops, n_cap);
		close(so_packet);
	} else
		fprintf(stderr, "[*] %d packet processed\n", n_cap);

	exit(i);
}

/*
 * usage
 */
void
usage()
{
	printf("%s [OPTIONS]:\n", __progname);
	printf(" input:\n");
	printf("   -i ifname        listen on interface\n");
	printf(" output:\n");
	printf("   -o file.out      output file. If not given stdout is used\n");
	printf(" socket:\n");
	printf("   -sr              use PF_PACKET:SOCK_RAW socket\n");
	printf("   -sd              use PF_PACKET:SOCK_DGRAM socket (mac removed)\n");
	printf(" filter protocol:\n");
	printf("   -fa              ETH_P_ALL (null filter)\n");
	printf("   -fi              ETH_P_IP\n");
#ifdef FRAME_FILTER
	printf(" filter frame:\n");
	printf("   -th              PACKET_HOST\n");
	printf("   -tb              PACKET_BROADCAST\n");
	printf("   -tm              PACKET_MULTICAST\n");
	printf("   -tp              PACKET_OTHERHOST\n");
	printf("   -to              PACKET_OUTGOING\n");
	printf("   -tl              PACKET_LOOPBACK\n");
	printf("   -tf              PACKET_FASTROUTE\n");
#endif
	printf(" dump:\n");
	printf("   -d0              tstamp framelen\n");
	printf("   -d1              tstamp framelen ipid tos\n");
#ifdef CACHE
	printf("   -d2              tstamp framelen ipid tos. cache=%dMb (raw)\n", CACHE);
	printf("   -d3              tstamp framelen ipid tos. cache=%dMb (raw:RX_RING)\n", CACHE);
	printf("   -d4              tstamp framelen ipid tos. cache=%dMb (raw:MACSTAT)\n", CACHE);
#else
	printf("   -d2              tstamp framelen ipid tos (raw)\n");
	printf("   -d3              tstamp framelen ipid tos (raw:RX_RING)\n");
	printf("   -d4              tstamp framelen ipid tos (raw:MACSTAT)\n");
#endif
	printf("   -d5              tstamp framelen flow_id seq_numb (rude)\n");
	printf(" others:\n");
	printf("   -P int           RR sched priority. (0=off..%d)\n", sched_get_priority_max(SCHED_RR));
	printf("   -p               set the promisc mode. (-i ifname is required)\n");
	printf("   -l n             polling no-blocking socket for n seconds\n");
	printf("   -r file          read raw lindump\n");
	printf("   -v               print version\n");
	printf("   -h               print this help\n");
	exit(1);
}


/*
 * Create socket and allign structures to rcv buffer
 */
int
create_socket()
{
	int val = 1, ret;

	switch (opt_socket) {
	case OPT_SOCK_RAW:
		ret = SOCKET(PF_PACKET, SOCK_RAW, htons(eth_p));
		iph = (struct iphdr *) (buffer + ETH_HLEN);
		udph = (struct udphdr *) (buffer + ETH_HLEN + sizeof(struct iphdr));
		datah = (struct udp_data *) (buffer + ETH_HLEN + sizeof(struct iphdr) + sizeof(struct udphdr));
		break;
		/* without datalink header */
	case OPT_SOCK_DGRAM:
		ret = SOCKET(PF_PACKET, SOCK_DGRAM, htons(eth_p));
		iph = (struct iphdr *) (buffer);
		udph = (struct udphdr *) (buffer + sizeof(struct iphdr));
		datah = (struct udp_data *) (buffer + sizeof(struct iphdr) + sizeof(struct udphdr));
		break;
	default:
		fatal("socket error");
	}

	if (opt_dump != OPT_DUMP_MACSTAT)
		return ret;

	if ((setsockopt(ret, SOL_PACKET, PACKET_MACSTAT,
			(char *) &val, sizeof(int))) == -1)
		fatal("[!] kernel lacks PACKET_MACSTAT sockopt. Apply af_packet-2.4.21 patch");

	return ret;
}


/*  -------------------  Setup ring  ------------------------

        ring                                  arena    block (page) = 16 frame per block
                                                     /
      +-------+                             +-------+
      |       |  ----------------------->   |       |
      +-------+                             |       |
      |       |  ----------------------->   |       |
      +-------+                             |       |
      |       |  ----------------------->   |       |
      +-------+                             |       |
      |       |  ----------------------->   |       |
      +-------+                             +-------+
      |       |                             |       |
      +-------+                             |       |
      |       |                             |       |
      +-------+                             |       |
      |       |                             |       |
      +-------+                             |       |
      |       |                             |       |
      +-------+                             +-------+ 2048 blocks
          .   \                                 .
          .     struct iovec                    .
          .                                     .

    ---------------------------------------------------------- */

#define TP_BLOCK_SIZE		4096
#define TP_FRAME_SIZE		256
#define TP_BLOCK_NR		2048
#define TP_FRAME_NR		TP_BLOCK_NR*(TP_BLOCK_SIZE/TP_FRAME_SIZE)
#define TP_FRAME_NR_MASK	TP_FRAME_NR-1

void
setup_ring()
{
	int i;

	tp_req.tp_block_size = TP_BLOCK_SIZE;	/* size of block */
	tp_req.tp_block_nr = TP_BLOCK_NR;	/* number of blocks */
	tp_req.tp_frame_size = TP_FRAME_SIZE;	/* size of frame */
	tp_req.tp_frame_nr = TP_FRAME_NR;	/* total number of frame: 16
						 * frames per block */

	if ((setsockopt(so_packet, SOL_PACKET, PACKET_RX_RING, (char *) &tp_req,
			sizeof(struct tpacket_req))) == -1)
		fatal("[!] kernel must be compiled with CONFIG_PACKET_MMAP!");

	arena = mmap(NULL, tp_req.tp_block_size * tp_req.tp_block_nr,
	      PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, so_packet, 0);

	if (arena == MAP_FAILED)
		fatal("mmap");

	/* setup the ring buffer */
	ring = (struct iovec *) malloc(tp_req.tp_frame_nr * sizeof(struct iovec));
	for (i = 0; i < tp_req.tp_frame_nr; i++) {
		ring[i].iov_base = (void *) ((long) arena) + (i * tp_req.tp_frame_size);
		ring[i].iov_len = tp_req.tp_frame_size;
	}
}


/* --- Filters --- */

#ifdef FRAME_FILTER
int
__frame_filter(int t)
{
	return !(f_filter & (1 << t));
}


int
__null_filter(int t)
{
	return 0;
}
#endif


static void
set_realtime(int prio)
{
	struct sched_param sp;
	memset(&sp, 0, sizeof(sp));
	sp.sched_priority = MIN(prio, sched_get_priority_max(SCHED_RR));
	sched_setscheduler(0, SCHED_RR, &sp);
	setpriority(PRIO_PROCESS, getpid(), -20);
}


int
main(int argc, char **argv)
{
	int i;

	/* defaults */
	opt_dump = OPT_DUMP_MIN;
	opt_socket = OPT_SOCK_RAW;
	eth_p = ETH_P_ALL;

#ifdef FRAME_FILTER
	while ((i = getopt(argc, argv, "t:f:r:l:po:d:s:P:vhi:")) != EOF)
#else
	while ((i = getopt(argc, argv, "f:r:l:po:d:s:P:vhi:")) != EOF)
#endif
		switch (i) {
#ifdef FRAME_FILTER
		case 't':
			switch (optarg[0]) {
			case 'h':
				f_filter |= OPT_FILTER_HOST;
				break;
			case 'b':
				f_filter |= OPT_FILTER_BROADCAST;
				break;
			case 'm':
				f_filter |= OPT_FILTER_MULTICAST;
				break;
			case 'p':
				f_filter |= OPT_FILTER_OTHERHOST;
				break;
			case 'o':
				f_filter |= OPT_FILTER_OUTGOING;
				break;
			case 'l':
				f_filter |= OPT_FILTER_LOOPBACK;
				break;
			case 'f':
				f_filter |= OPT_FILTER_FASTROUTE;
				break;
			default:
				fatal("unknown frame type");
			}
			break;
#endif
		case 's':
			switch (optarg[0]) {
			case 'r':
				opt_socket = OPT_SOCK_RAW;
				break;
			case 'd':
				opt_socket = OPT_SOCK_DGRAM;
				break;
			default:
				fatal("unknown socket method");
			}
			break;
		case 'd':
			switch (optarg[0]) {
			case '0':
				opt_dump = OPT_DUMP_MIN;
				break;
			case '1':
				opt_dump = OPT_DUMP_IPID;
				break;
			case '2':
				opt_dump = OPT_DUMP_RAW;
				break;
			case '3':
				opt_dump = OPT_DUMP_RING;
				break;
			case '4':
				opt_dump = OPT_DUMP_MACSTAT;
				break;
			case '5':
				opt_dump = OPT_DUMP_RUDE;
				break;
			default:
				fatal("unknown socket method");
			}
			break;
		case 'f':
			switch (optarg[0]) {
			case 'a':
				eth_p = ETH_P_ALL;
				break;
			case 'i':
				eth_p = ETH_P_IP;
				break;
			default:
				fatal("unknown filter protocol");
			}
			break;
		case 'i':
			dev_name = strdup(optarg);
			break;
		case 'r':
			input = strdup(optarg);
			opt_dump = OPT_DUMP_REPLAY;
			break;
		case 'l':
			options |= OPT_POLL;
			tim = atoi(optarg);
			break;
		case 'P':
			priority = atoi(optarg);
			break;
		case 'o':
			output = strdup(optarg);
			break;
		case 'v':
			fprintf(stderr, "%s\n", cvsid);
			exit(1);
		case 'p':
			options |= OPT_PROM;
			break;
		case 'h':
			usage();
		case '?':
			fatal("opt: unknown option");
			break;
		}

	argc -= optind;
	argv += optind;

	fprintf(stderr, "[*] %s\n", cvsid);

	/* -- uid -- */
	if (getuid() != 0)
		fatal("[!] %s must run as root", __progname);

	/* -- dependencies -- */
	if (((opt_dump == OPT_DUMP_RAW) ||
	     (opt_dump == OPT_DUMP_RING) ||
	     (opt_dump == OPT_DUMP_MACSTAT)) && !strcmp(output, "stdout"))
		fatal("[!] raw dump requires -o filename");

	/* open files */
	fprintf(stderr, "[w] output: open(%s)\n", output);

	if (strcmp(output, "stdout")) {
		fdout = open(output, O_WRONLY | O_CREAT | O_EXCL, 0644);
		if (fdout == -1)
			fatal("can't open %s for writing", output);

		fileout = fdopen(fdout, "w");
		if (fileout == NULL)
			fatal("can't open stream from fd=%d", fdout);
	} else
		fileout = stdout;

	if (input != NULL) {
		fprintf(stderr, "[r] input: open(%s)\n", input);
		if ((filein = fopen(input, "r")) == NULL)
			fatal("can't open %s", input);
		opt_dump = OPT_DUMP_REPLAY;
	}

	if (opt_dump == OPT_DUMP_MACSTAT && opt_socket == OPT_SOCK_DGRAM) {
		/* OPT_DUMP_MACSTAT needs SOCK_RAW */
		fprintf(stderr, "    PACKET_MACSTAT -> forcing SOCK_RAW.\n");
		opt_socket = OPT_SOCK_RAW;
	}


	/* open socket */
	if (opt_dump != OPT_DUMP_REPLAY) {
		so_packet = create_socket();
		if (so_packet == -1)
			fatal("socket error");

        	/* set up ring */
        	if (opt_dump == OPT_DUMP_RING)
                	setup_ring();

		/* filters */
		switch (eth_p) {
		case ETH_P_ALL:
			fprintf(stderr, "    if_ether protocol filter: ETH_P_ALL(0x%.4x)\n", eth_p);
			break;
		case ETH_P_IP:
			fprintf(stderr, "    if_ether protocol filter: ETH_P_IP(0x%.4x)\n", eth_p);
			break;
		}

		/* bind socket to the interface */
		if (dev_name != NULL) {
			/* bind ll_socket to interface */
			struct sockaddr_ll sll;
			memset(&sll, 0, sizeof(sll));
			sll.sll_family = AF_PACKET;
			sll.sll_ifindex = iface_getid(so_packet, dev_name);
			sll.sll_protocol = htons(eth_p);

			if (bind(so_packet, (struct sockaddr *) & sll, sizeof(sll)) == -1)
				fatal("bind()");

			fprintf(stderr, "    listening on device %s (id=%p)\n", dev_name, index);

		} else
			fprintf(stderr, "    listening on ALL devices (*)\n");

		/* set promisc */
		if (dev_name != NULL && (options & OPT_PROM)) {
			iface_setprom(so_packet, dev_name);
		}
		/* alarm */
		if (options & OPT_POLL) {
			alarm(tim);
			set_nonblock(so_packet);
		}
#ifdef FRAME_FILTER
		if (f_filter)
			frame_filter = __frame_filter;
		else
			frame_filter = __null_filter;
#endif
#ifdef CACHE
		/* alloc big_buffer */
		if (	opt_dump == OPT_DUMP_RAW     ||
			opt_dump == OPT_DUMP_MACSTAT || 
			opt_dump == OPT_DUMP_RING 	) {
			big_buffer = (char *) malloc(CACHE mbyte);
			big_limit = big_buffer + (CACHE mbyte);
			if (big_buffer != NULL)
				fprintf(stderr, "    Allocated %d Mbyte cache memory.\n", CACHE);
			else
				fatal("buffer error");
		}
#endif
	}

	/* setup values */
	signal(SIGINT, exit_handler);
	signal(SIGALRM, exit_handler);

	/* set round-robin scheduling policy */
	if (priority > 0) {
		fprintf(stderr, "    Round Robin scheduling with priority %d.\n", priority);
		set_realtime(priority);
	}

	/* run the proper dumper */
	dumper();
	exit(0);
}
