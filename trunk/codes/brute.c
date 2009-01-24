/*
      This source code is deprecated. The brute project is currently hosted at:
      http://netgroup-serv.iet.unipi.it/brute
  
      Nicola
*/
/*
 * brute:    High Performance UDP generator for Linux 2.4. 
 *           Benchmarking Methodology for Network Interconnect Devices. [rfc2544 compliant]
 *   
 * Testbed:  Intel(R) Pentium(R) 4 CPU 2.40GHz bogomips: 4784.26
 *           ethernet 0: 3com Corporation 3c905C-TX/TX-M [Tornado] (rev 74)
 *           ethernet 1: Intel PRO/100
 *           ethernet 2: 3com Corporation gigabit fiber-SX (3C996-SX)
 *           ethernet 3: Intel PRO/1000 MF (Sever Adapter LX)
 *
 *           Test with Adtech AX/4000 Broadband Test System (Traffic Analyzer)
 *          
 *          frame lenght  |  max rate    |      link       |    comment
 *       -----------------+--------------+-----------------+-----------------
 *   (*) rude  @64 bytes  | 70.000  fps  | eth0:100  mbps  |
 *       brute @64 bytes  | 140.000 fps  | eth0:100  mbps  | ~ cable capacity 
 *       brute @64 bytes  |     ?        | eth1:100  mbps  |
 *       brute @64 bytes  | 267.000 fps  | eth2:1000 mbps  | with no dups
 *       bryte @64 bytes  |     ?        | eth3:1000 mbps  |
 *    
 *       rude is: http://rude.sourceforge.net. 
 * 
 * compile: gcc -O6 -march=arch_type brute.c -o brute
 *
 * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
 *                                   <bonelli@netserv.iet.unipi.it>
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
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/ether.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <features.h>		/* for the glibc version number */
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>	/* the L2 protocols */
#else
#include <asm/types.h>
#include <asm/msr.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>	/* The L2 protocols */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>

static char cvsid[] = "$Id: brute.c,v 1.42 2003/12/05 12:56:27 awgn Exp $";

/* usage string */

static char usage_str[]="\
%s [OPTIONS]:                                           \n\
 mac:                                                   \n\
   -d MAC|rand      dst mac address.                    \n\
   -s MAC           src mac address.                    \n\
 scheduler:                                             \n\
   -p int           RR sched priority. (0=off..%d)      \n\
 others:                                                \n\
   -r1              dest. ip, 256 random host           \n\
   -r2              dest. ip, 256 random network        \n\
   -i ifname        interface                           \n\
   -f script        brute file                          \n\
   -v               print version                       \n\
   -h               print this help                     \n";


/*
 * Macro
 */


#ifndef USE_DUP
#warning "-DUSE_DUP exploits af_packet-2.4.21 patch, allowing dup packets"
#endif

#define OPT_RANDMAC		1<<0
#define OPT_RANDIP		1<<1
#define OPT_EHINCL		1<<2	/* ethernet header included */ 
#define OPT_MACSRC		1<<3
#define OPT_MACDST		1<<4

#define MAX_FLOWSTEP	1024	
#define PARSE_BUFFLEN   1024
#define PARSE_BUFFCOM   80

#define DEV_URANDOM	"/dev/urandom"
#define CPUINFO		"/proc/cpuinfo"
#define CPUBUFF 	1024

#define SOCKET(x,y,z)  ({ 					\
int _i;								\
_i=socket(x,y,z); 						\
fprintf(stderr,"[+] create: socket(%s,%s,%s)\n",#x,#y,#z);	\
_i;})

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#define U(x)     ( (x) > 0 ? x  : 0 )
#define ABS(x)   ( (x) < 0 ? -(x) : (x) )

#define COMM_FLOW	1
#define COMM_TOUCH	2

#ifndef PACKET_DUP
#define PACKET_DUP	9
#endif

typedef struct {
	u_int comm;		/* command type */
	u_int msec;
	u_int len;
	u_int rate;
	u_int dup;
	u_long s_host;
	u_long d_host;
	u_short s_port;
	u_short d_port;
	u_char ttl;
	u_char tos;
}  STEP;


/*
 * global
 */


STEP flows[MAX_FLOWSTEP];

extern char *__progname;

char arena[ETH_FRAME_LEN];	/* 1514 bytes: enough for an ethernet frame */

char *filedat;
char *src_mac;
char *dst_mac;
char *ifname;

struct sockaddr_ll sock_ll;
struct sched_param sched;

int priority=99;		/* highest priority by default */
int options;
int sock;
int flow_index;
int ether_offset;
int urand;
int rand_class;			/* dest-ip random class: C=1, B=2 */

struct ethhdr *ethh = (struct ethhdr *) arena;
struct iphdr *iph;
struct udphdr *udph;
char *udp_data;

u_short old_chksum;
u_long  ip_id = 1; 

char parse_buff[PARSE_BUFFLEN];
char parse_comm[PARSE_BUFFCOM];
char cpubuff[CPUBUFF];

char src_host[256];
char dst_host[256];

unsigned long long Hz;

void (*update_arena)();
void (*touch_arena)();

/*
 * Fucntions
 */

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


unsigned long
getlongbyname(char *host)
{
	struct in_addr addr;
	struct hostent *host_ent;

	if ((addr.s_addr = inet_addr(host)) == -1) {

		if ((host_ent = gethostbyname(host)) == NULL) {
			fatal("gethostbyname() or inet_addr() err:%s\n", strerror(errno));
		}
		memcpy((char *) &addr.s_addr, host_ent->h_addr, host_ent->h_length);
	}
	return addr.s_addr;
}


unsigned long long
get_hz()
{
        double MHz;
        char *p = cpubuff;
        int i;

        if ( (i=open(CPUINFO,O_RDONLY)) == -1)
                fatal("%s open error",CPUINFO);
        read(i,cpubuff,CPUBUFF);
        close(i);

        for (i=0;i<7;i++,p++)
                p=(char *)strchr(p,':');
        p++;

        if( (i=sscanf(p,"%lf\n",&MHz)) != 1 )
                fatal("%s open error",CPUINFO);

        return MHz*1000000;
}


void post_exit() __attribute__((noreturn, destructor));
void
post_exit()
{
	static int i;

	if (i++ != 0)
		exit (0);

	if (sock != 0) {
		close(sock);
		fprintf(stderr, "[*] %lu sendto() done.\n",ip_id-1);
	}
	exit(0);
}


/*
 * return the udp_datalen.
 */
int
udp_datalen(char t, int len)
{
	switch (t) {
	case 'u':
		return len;
	case 'f':
		return len-ETH_HLEN-4-sizeof(struct iphdr)-sizeof(struct udphdr);	
	default:
		fatal("udp_datalen: error");
	}
} 


/*
 * lame parser for brute.data file
 */
void
brute_parse(char *file)
{
	FILE *f;
	int ret, s_port, d_port, msec, len, rate, tos, dup, ttl = -1;
	int i = -1, flow_cnt = 0;
	char t_len;

	fprintf(stderr, "[r] input: open(%s)\n", file);

	f = fopen(file, "r");
	if (f == NULL)
		fatal("can't open %s", file);

	for (;;) {

		if (flow_cnt == MAX_FLOWSTEP)
			break;

		if (fgets(parse_buff, PARSE_BUFFLEN, f) == NULL)	/* EOF */
			break;
		i++;
		s_port = d_port = msec = len = rate = tos = dup = 0;
		ttl = -1;

		ret = sscanf(parse_buff, "%79s %d %c:%d %d %*[x]%d %255[^:]:%d %255[^:]:%d %x %d",
			     parse_comm, &msec, &t_len, &len, &rate, &dup, src_host, &s_port, dst_host, &d_port, &tos, &ttl);

		/* --- skip: empty line or comment --- */

		if (ret == -1 || parse_comm[0] == '#' || parse_comm[1] == '#')
			continue;

		if (t_len != 'u' && t_len != 'f')
			fatal("brute_parse:%d unknown type '%c' lenght.",i+1,t_len);

		/* --- check the list of known command --- */
		
		/* command: flow */
		if (ret >= 10 && strcmp("flow", parse_comm) == 0) {
			flows[flow_cnt].comm = COMM_FLOW;
			flows[flow_cnt].s_host = getlongbyname(src_host);
			flows[flow_cnt].d_host = getlongbyname(dst_host);
			flows[flow_cnt].s_port = (u_short) s_port;
			flows[flow_cnt].d_port = (u_short) d_port;
			flows[flow_cnt].msec = ABS(msec);
			flows[flow_cnt].len = udp_datalen(t_len,ABS(len));
			flows[flow_cnt].rate = ABS(rate);
			flows[flow_cnt].tos = (u_char) tos;
			flows[flow_cnt].ttl = (u_char) (ttl == -1 ? 64 : ttl);
			flows[flow_cnt].dup = dup;
			flow_cnt++;
			continue;
		}
		/* command: touch */
		if (ret == 6 && strcmp("touch", parse_comm) == 0) {
			flows[flow_cnt].comm = COMM_TOUCH;
			flows[flow_cnt].s_host = 0;
			flows[flow_cnt].d_host = 0;
			flows[flow_cnt].s_port = 0;
			flows[flow_cnt].d_port = 0;
			flows[flow_cnt].msec = ABS(msec);
			flows[flow_cnt].len = udp_datalen(t_len,ABS(len));
			flows[flow_cnt].rate = ABS(rate);
			flows[flow_cnt].dup = dup;
			flow_cnt++;
			continue;
		}
		/* --- add other commands here --- */

		/* --- end of commands --- */

		fatal("brute_parse:%d \"%s\" command error.", i+1, parse_comm);
	}
	printf("[+] parsed %d lines, %d flows load.\n", i, flow_cnt);
	return;
}

__inline
void
lineup_headers()
{
	if (options & OPT_EHINCL)
		ether_offset = ETH_HLEN; 
 
	iph = (struct iphdr *)   (arena + ether_offset);
	udph = (struct udphdr *) (arena + ether_offset + sizeof(struct iphdr));
        udp_data = (char *)      (arena + ether_offset + sizeof(struct iphdr)+ sizeof(struct udphdr));
}


/*
 * compute an IP header checksum.
 * don't modify the packet.
 */
u_short
in_chksum(const u_short *addr, register u_int len)
{
        int nleft = len;
        const u_short *w = addr;
        u_short answer;
        int sum = 0;

        /*
         *  Our algorithm is simple, using a 32 bit accumulator (sum),
         *  we add sequential 16 bit words to it, and at the end, fold
         *  back all the carry bits from the top 16 bits into the lower
         *  16 bits.
         */
        while (nleft > 1)  {
                sum += *w++;
                nleft -= 2;
        }
        if (nleft == 1)
                sum += htons(*(u_char *)w<<8);

        /*
         * add back carry outs from top 16 bits to low 16 bits
         */
        sum = (sum >> 16) + (sum & 0xffff);     /* add hi 16 to low 16 */
        sum += (sum >> 16);                     /* add carry */
        answer = ~sum;                          /* truncate to 16 bits */
        return answer;
}


/*  
 * fast internet checksum update: ftp://ftp.rfc-editor.org/in-notes/rfc1624.txt 
 */
__inline
u_short
fast_chksum(u_short old)
{

//
//      Given the following notation:
//
//      HC  - old checksum in header
//      C   - one's complement sum of old header
//      HC' - new checksum in header
//      C'  - one's complement sum of new header
//      m   - old value of a 16-bit field
//      m'  - new value of a 16-bit field
//
//
//      HC' = ~(~HC + ~m + m')	
//
// 	register int sum;
//
//	sum  = ~ntohs(old) & 0xffff;
//	sum  += ~(ip_id-1)	  & 0xffff;
//	sum  += ip_id;
//      sum  = (sum >> 16) + (sum & 0xffff);    /* add hi 16 to low 16 */
//      sum += (sum >> 16);     		/* add carry */
//	old_chksum  = htons((u_short)~sum);
//
//      In this case, whereas only ip_id++ changes, we have:
//
//               .--- 0  : if m'=0xffff and m=0 
//  (*) ~m+ m'= /
//              \ 
//               `--- 1  : otherwise           
//
//      (*) the sum is computed with 1's complement math.
//
//      This let to simplify the algorithm as follows:

	register int sum;
#if __BYTE_ORDER == __LITTLE_ENDIAN
	sum  = old - ( (u_short)ip_id == 0 ? 0 : 0x100 );
	sum -= ( sum & 0xffff0000 ? 1 : 0 );
#elif __BYTE_ORDER == __BIG_ENDIAN
	sum  = old - ( (u_short)ip_id == 0 ? 0 : 0x1 );
#else
#error  "fast_chksum() not implemented for your endianess"
#endif
	return sum & 0xffff;
}


/*
 * Fill ip/udp headers
 */
void
__setup_arena()
{
	/* --- compile udp header --- */

	udph->source = htons(flows[flow_index].s_port);
	udph->dest = htons(flows[flow_index].d_port);
	udph->len = htons(flows[flow_index].len);
	udph->check = 0;	/* the udp cheksum is not required */

	/* --- compile ip header --- */

	iph->version = 4;	/* ipv4 */
	iph->ihl = 5;		/* ip header len in words */
	iph->tos = flows[flow_index].tos;
	iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + flows[flow_index].len);
	iph->id = htons((u_short)ip_id);
	iph->frag_off = htons(IP_DF);
	iph->ttl = flows[flow_index].ttl;
	iph->protocol = IPPROTO_UDP;

	iph->saddr = (flows[flow_index].s_host);
	iph->daddr = (flows[flow_index].d_host);
	iph->check = 0;
	iph->check = old_chksum = in_chksum((u_short *) iph, iph->ihl << 2);
}


/*
 * change udph->len, iph->tot_len and iph->check
 */
void
__touch_arena()
{
	/* --- udp changes --- */
	udph->len = htons(flows[flow_index].len);

	/* --- ip changes --- */
	iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + flows[flow_index].len);
	iph->check = 0;
	iph->check = old_chksum = in_chksum((u_short *) iph, iph->ihl << 2);
}


void
__touch_arena_mac()
{
	read(urand, ethh->h_dest, ETH_ALEN);    /* rand mac */

        /* --- udp changes --- */
        udph->len = htons(flows[flow_index].len);

        /* --- ip changes --- */
        iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + flows[flow_index].len);
        iph->check = 0;
        iph->check = old_chksum = in_chksum((u_short *) iph, iph->ihl << 2);
}


void
__touch_arena_ip()
{

#if __BYTE_ORDER == __LITTLE_ENDIAN
	switch(rand_class) {
		case 1:
			read(urand, 3+(void *)&iph->daddr, 1);
			break;
		case 2:
			read(urand, 2+(void *)&iph->daddr, 2);
			break;
	}
#elif __BYTE_ORDER == __BIG_ENDIAN
	read(urand, &iph->daddr, rand_class);    /* rand ip */
#else
#error  "__touch_arena_ip() not implemented for your endianess"
#endif

        /* --- udp changes --- */
        udph->len = htons(flows[flow_index].len);

        /* --- ip changes --- */
        iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + flows[flow_index].len);
        iph->check = 0;
        iph->check = old_chksum = in_chksum((u_short *) iph, iph->ihl << 2);
}


/*
 * change only iph->id, recompute the chucksum using fast_chksum algorithm.
 */
void
__update_arena()
{
	iph->id = htons((u_short)ip_id);
	iph->check = 0;
	iph->check = old_chksum = fast_chksum(old_chksum);
}


void
__update_arena_mac()
{
	read(urand, ethh->h_dest, ETH_ALEN);	/* rand mac */
        iph->id = htons((u_short)ip_id);
        iph->check = 0;
        iph->check = old_chksum = fast_chksum(old_chksum);
}


void
__update_arena_ip()
{

#if __BYTE_ORDER == __LITTLE_ENDIAN
        switch(rand_class) {
                case 1:
                        read(urand, 3+(void *)&iph->daddr, 1);
                        break;
                case 2:
                        read(urand, 2+(void *)&iph->daddr, 2);
                        break;
        }
#elif __BYTE_ORDER == __BIG_ENDIAN
        read(urand, &iph->daddr, rand_class);    /* rand ip */
#else
#error  "__update_arena_ip() not implemented for your endianess"
#endif

        iph->id = htons((u_short)ip_id);
        iph->check = 0;
	iph->check = old_chksum = in_chksum((u_short *) iph, iph->ihl << 2);
}


void
setup_arena()
{
	static int flow_set;

	if (flow_set == 0 && flows[flow_index].comm == COMM_TOUCH)
		fatal("flow command required");

	if (flows[flow_index].comm == 0)
		exit(0);

	switch (flows[flow_index].comm) {
	case 0:		/* the end */
		exit(0);
	case COMM_FLOW:
		old_chksum = 0;	/* reset chksum */
		flow_set = 1;
		__setup_arena();
		break;
	case COMM_TOUCH:
		touch_arena();
		break;
	default:
		fatal("unknown command type");
	}
}


/*
 * get the interface id by name.
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
 * create the PF_PACKET socket, and setup ether header.
 */
int
create_socket(char *ifname, char *s_mac, char *d_mac)
{
	struct ether_addr *s_MAC, *d_MAC;
	int ret;

	if (s_mac == NULL)
		ret = SOCKET(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	else
		ret = SOCKET(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (ret == -1)
		fatal("socket");

	sock_ll.sll_family = AF_PACKET;
	sock_ll.sll_halen = ETH_ALEN;	/* MAC */
	sock_ll.sll_protocol = htons(ETH_P_IP);
	sock_ll.sll_ifindex = iface_getid(ret, ifname);

	if (options & OPT_EHINCL) {
		s_MAC = ether_aton(s_mac);
		memcpy(ethh->h_source, s_MAC->ether_addr_octet, ETH_ALEN);

		if ( options & OPT_MACDST) {
			d_MAC = ether_aton(d_mac);
			memcpy(ethh->h_dest, d_MAC->ether_addr_octet, ETH_ALEN);
		} else
			read(urand, ethh->h_dest, ETH_ALEN);
	
		ethh->h_proto = htons(ETH_P_IP);

	} else {
		if ( (d_MAC = ether_aton(d_mac))== NULL)
			fatal("destination MAC, wrong format");	
		memcpy(sock_ll.sll_addr, d_MAC->ether_addr_octet, ETH_ALEN);		 
	}

	return ret;
}


/*
 * print the current flow
 */
void
dump_flow()
{
	static struct in_addr src, dst;
	static short sport, dport;
	static u_char tos, ttl;

	/* --- setup step --- */

	if (flows[flow_index].comm == COMM_FLOW) {
		src.s_addr = flows[flow_index].s_host;
		dst.s_addr = flows[flow_index].d_host;
		sport = flows[flow_index].s_port;
		dport = flows[flow_index].d_port;
		tos = flows[flow_index].tos;
		ttl = flows[flow_index].ttl;
	}
	fprintf(stderr, " `--> flow %d: %s:%d ---> %s:%d len=%d tos=0x%2x ttl=%d\n", flow_index,
		strdup(inet_ntoa(src)), sport, strdup(inet_ntoa(dst)), dport,
		flows[flow_index].len, tos, ttl);
}



__inline
void
wait_xmit(unsigned long long *t)
{
	register unsigned long long n;
	do { rdtscll(n); }
		while ( n < *t );
}


/*
 * udp generator engine
 */
void
udp_generator()
{
	unsigned long long init, now, tick, tau, step, norm;
	u_int sent, size;
	register int len;

	size = sizeof(struct sockaddr_ll);

	rdtscll(init);

	for ( ; flows[flow_index].comm != 0; flow_index++ ) {

		rdtscll(init);
		dump_flow();
		setup_arena();
		
		len  = ether_offset + sizeof(struct iphdr) + sizeof(struct udphdr) + flows[flow_index].len;
		step = (unsigned long long)((Hz/flows[flow_index].rate));
		tau  = init+(unsigned long long)((Hz/1000*flows[flow_index].msec));
		norm = (unsigned long long)(flows[flow_index].rate)*flows[flow_index].msec/1000;
		sent = ip_id;
		tick = init + step;

		rdtscll(now);	

#ifdef USE_DUP
        	if (	flows[flow_index].dup > 1 && 
		   (setsockopt(sock, SOL_PACKET, PACKET_DUP, (char *)&flows[flow_index].dup , sizeof(int))) == -1)
                	fatal("[!] kernel lacks PACKET_DUP sockopt in order to send dup frames. Apply af_packet-2.4.21 patch");
#endif

		for ( ; now < tau; tick += step ) {
			if(sendto(sock, arena, len, 0, (struct sockaddr *) &sock_ll, size)!=-1)
				ip_id++ , update_arena();
			wait_xmit(&tick);
			rdtscll(now);
		}
#ifdef USE_DUP
		printf("      sendto()=%u/%u x%u +=%.2f%% (jitter)\n",
			(u_int)ip_id-sent, (u_int)norm, flows[flow_index].dup, (double)(ip_id-sent)*100/norm-100.0); 
#else
                printf("      sendto()=%u/%u +=%.2f%% (jitter)\n",
                        (u_int)ip_id-sent, (u_int)norm, (double)(ip_id-sent)*100/norm-100.0);

#endif
					       
	}
}


static void
set_realtime (int prio)
{
    	struct sched_param  sp;

	fprintf(stderr, "[*] Round Robin scheduling with priority %d.\n", prio);
    	memset (&sp, 0, sizeof(sp) );
    	sp.sched_priority = MIN (prio, sched_get_priority_max(SCHED_RR));
    	sched_setscheduler ( 0, SCHED_RR, &sp );
    	setpriority ( PRIO_PROCESS, getpid(), -20 );
}


int
main(int argc, char **argv)
{
	int i;

	while ((i = getopt(argc, argv, "hvf:i:d:s:p:r:")) != EOF)
		switch (i) {
		case 'r':
			options |= OPT_RANDIP;
			switch(optarg[0]) {
				case '1':
					rand_class=1;
					break;
				case '2':
					rand_class=2;
					break;
				default:
					fatal("unknown rand class opt");
			}
			break;
		case 'd':
			if ( strcasecmp(optarg,"rand") == 0 )
				options |= (OPT_RANDMAC|OPT_EHINCL);
			else {
				dst_mac = strdup(optarg);
				options |= (OPT_MACDST);
			}

			break;
		case 's':
			options |= (OPT_MACSRC|OPT_EHINCL);
			src_mac = strdup(optarg);
			break;
		case 'p':
			priority = atoi(optarg);
			break;
		case 'f':
			filedat = strdup(optarg);
			break;
		case 'i':
			ifname = strdup(optarg);
			break;
		case 'v':
			fprintf(stderr, "%s\n", cvsid);
			exit(1);
		case 'h':
        		printf(usage_str,__progname,sched_get_priority_max(SCHED_RR));
        		exit(1);        
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

	if (filedat == NULL)
		fatal("[!] brute file not given");

	if ( !(options & (OPT_MACDST|OPT_RANDMAC)) )
		fatal("[!] destination MAC address not given");

	if ( (options & OPT_EHINCL) && !(options & OPT_MACSRC) )
		fatal("[!] source MAC is required when using random destination MAC");

	if (ifname == NULL)
		fatal("[!] ifname not given");

	if (options & OPT_RANDMAC && options & OPT_RANDIP)
		fatal("[!] randmac and randip options are mutually exclsuive \n");

	if (options & (OPT_RANDMAC|OPT_RANDIP)) {
		urand = open(DEV_URANDOM,O_RDONLY); 
		if ( urand == -1 )
			fatal("open(\"%s\") error",DEV_URANDOM);
	}

	/* update_arena */
	if ( options & OPT_RANDIP  ) {
		 update_arena = __update_arena_ip;
		 touch_arena  = __touch_arena_ip;
		 goto fun;
	}

	if ( options & OPT_RANDMAC ) {
		update_arena = __update_arena_mac;
		touch_arena  = __touch_arena_mac;
		goto fun;
	}

	/* default handlers */
	update_arena = __update_arena;
	touch_arena  = __touch_arena;
fun:

	/* parsing filedat */
	brute_parse(filedat);

	/* signal */
	signal(SIGINT, post_exit);

	/* lineup headers */
        lineup_headers();

	/* create datalink socket */
	sock = create_socket(ifname, src_mac, dst_mac);

        /* reset arena: frame format rfc2544 */
	for (i=0; &udp_data[i] < &arena[ETH_FRAME_LEN]; i++)       
                udp_data[i]=(char)i;

	/* build the packet ip */
	setup_arena();

        /* Get Mhz */
        Hz = get_hz();
        fprintf(stderr, "[i] cpu MHz: %lld.%lld\n", Hz/1000000,Hz%1000000);

	fprintf(stderr, "[+] ether_offset: %d\n", ether_offset);

        /* set round-robin scheduling policy */
	if (priority)
		set_realtime(priority);		

	/* launch flooder */
	udp_generator();

	return 0;
}
