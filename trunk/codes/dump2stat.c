/*
 * dump2stat: convert tcpdump, lindump, crude flows (timestamps,frame-len) into
 *            gnuplotable throughput/loss graph for DUT analisys.
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
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

static char cvsid[] = "$Id: dump2stat.c,v 1.36 2004/04/15 10:59:46 awgn Exp $";

/* unit util */
#define bit(x)         ((x)>>3 )
#define kbit(x)        ((x)<<7 )
#define mbit(x)        ((x)<<17)
#define gbit(x)        ((x)<<27)
#define kbyte(x)       ((x)<<10)
#define mbyte(x)       ((x)<<20)
#define gbyte(x)       ((x)<<30)

/* parser */
#define OPT_TCPDUMP	1	/* http://www.tcpdump.org */
#define OPT_CRUDE	2	/* http://rude.sourceforge.net */
#define OPT_LINDUMP	3	/* http://awgn.antifork.org/codes/lindump.c */

/* output */
#define OPT_BPS		1<<0	/* bit per second (net)   */
#define OPT_GROSS	1<<1	/* bit per second (gross) */
#define OPT_FPS		1<<2	/* frame per second */
#define OPT_LOSS	1<<3
#define OPT_LOSSP	1<<4
#define OPT_LOSSRATE	1<<5
#define OPT_REALIGN	1<<6

#define SLIDE_BYTES     0
#define SLIDE_FRAMES    1
#define SLIDE_LOST	2
#define SLIDE_LOSTP	3

#define BUFFLEN		1024

#define PREAMBLE	bit(64)
#define DATALINK        14+4    /* 14 of header, 4 of ethernet crc */
#define GAP 		bit(96)

#define IP		20
#define UDP		8
#define WRAPAROUND	256
#define MAX_FLOWSTEP	1024	/* brute flows */

/* parser options */
#define  PARSE_NOFILTER	0
#define	 PARSE_FILTER	1
 
/* lindump filter */
#include <linux/if_packet.h>
#define OPT_FILTER_HOST         (1<<PACKET_HOST)
#define OPT_FILTER_BROADCAST    (1<<PACKET_BROADCAST)
#define OPT_FILTER_MULTICAST    (1<<PACKET_MULTICAST)
#define OPT_FILTER_OTHERHOST    (1<<PACKET_OTHERHOST)
#define OPT_FILTER_OUTGOING     (1<<PACKET_OUTGOING)
#define OPT_FILTER_LOOPBACK     (1<<PACKET_LOOPBACK)
#define OPT_FILTER_FASTROUTE    (1<<PACKET_FASTROUTE)

/* brute script commands */

#define COMM_FLOW	1
#define COMM_TOUCH	2


unsigned long f_filter;		/* lindump frame filter */
unsigned long t_filter;		/* lindump tos filter */

unsigned long lut_filter[256]= {
['<'] = OPT_FILTER_HOST, 
['B'] = OPT_FILTER_BROADCAST, 
['M'] = OPT_FILTER_MULTICAST, 
['O'] = OPT_FILTER_OTHERHOST, 
['>'] = OPT_FILTER_OUTGOING, 
['L'] = OPT_FILTER_LOOPBACK, 
['F'] = OPT_FILTER_FASTROUTE, 
};

unsigned char lut_tos[256];

/*
 * Macro
 */
#define REG_BS(r,b)  ( r.reg[b>>3] |=   1<<(b&7) )
#define REG_BR(r,b)  ( r.reg[b>>3] &= ~ 1<<(b&7) )
#define REG_BT(r,b)  ( r.reg[b>>3]  &   1<<(b&7) )
#define REG_BN(r,b)  ( r.reg[b>>3] ^=   1<<(b&7) )

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )
#define MAX(a,b) ( (a) > (b) ? (a) : (b) )
#define U(x)     ( (x) > 0 ? x  : 0 )
#define ABS(x)   ( (x) < 0 ? -(x) : (x) )

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


struct frame {
	unsigned long tv_sec;
	unsigned long tv_usec;
	int len;		/* =0 -> empty frame. = -1 -> lost frame */
	u_short id;
	struct frame *next;
};

struct timeval origin;

/* global vars */

u_int parse_opt;
u_int output_opt;
u_int winsize;			/* smooth windows: usec */
u_int res;			/* resolution in usec */

u_int tare;
u_int flow_id;
u_int last_id;

int (*sliding_win) (struct frame *, double *, int *, int *);

int (*parser) (struct frame *, FILE *, int);

char buffer[BUFFLEN];

struct frame *head;
struct frame *last;

char *src_host;
char *dst_host;

char *dumpfile[4] = {[OPT_TCPDUMP] = "TCPDUMP",[OPT_CRUDE] = "CRUDE", [OPT_LINDUMP]="LINDUMP"};
char *outtype[6] = {[0] = "Mbps (net)",[1] = "Mbps (gross)",[2] = "fps (frame/sec)",
[3] = "loss (fps)",[4] = "loss %",[5]="loss vs rate"};

char *fileout = "flow.out";
char *filein = "stdin";
char *filebrute;

/* loss data */

typedef struct {
	unsigned char reg[WRAPAROUND >> 3];
}      REG;

REG dut_id[1 << 16];			/* ip_id tables for DUT output */

STEP flows[MAX_FLOWSTEP];	 	/* brute flows script */
long long counter[MAX_FLOWSTEP]; 	/* counter for each flows */

/* Functions */

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

/*
 *  normalize OPT_bit in integer sequence.
 */

int
norm(int i)
{
	int ret;
	for (ret = 0; (i >>= 1); ret++)
		continue;
	return ret;
}

/* --- Tcpdump -ttv parser --- */

int
parse_tcpdump(struct frame * p, FILE * in, int mode)
{
	char src[80], dst[80];
	int sec, usec, len, id, tos, ret;
	char *ptr;

	if (fgets(buffer, BUFFLEN, in) == NULL)
		return -1;	/* EOF */

	ret = sscanf(buffer, "%d.%d %80s > %80s", &sec, &usec, src, dst);

//	if (ret != 4 || (ptr = strstr(buffer, "[tos")) == NULL)
	if (ret != 4 || (ptr = strstr(buffer, "(")) == NULL )
		return 0;	/* skip this packet */

	if (dst[strlen(dst) - 1] == ':')
		dst[strlen(dst) - 1] = '\0';

	/* filters */

//        if (sscanf(ptr, "[tos %x]  (ttl %*d, id %d, len %d", &tos,&id, &len) != 3)
	{
	int ret;
	if ( (ret=sscanf(ptr, "(%x) (ttl %*d, id %d, len %d", &tos,&id, &len)) != 3)
                fatal("[%s]: parse error! (%d)", buffer,ret);
	}

	if (!mode)
		goto post_filter;

	if ((src_host && strcmp(src_host, src)) ||
	    (dst_host && strcmp(dst_host, dst)))
		return 0;	/* src/dst host filters */

        /* tos filter */
        if ( (t_filter & lut_tos[tos&0xff]) == 0 )
                return 0;

post_filter:
	p->len = len + tare + DATALINK;
	p->tv_sec = sec;
	p->tv_usec = usec;
	p->id = id;

	return 1;		/* OK */
}


/* --- Crude dump parser --- */

int
parse_crude(struct frame * p, FILE * in, int mode)
{
	int id, sec, usec, len;
	int ret;

	if (fgets(buffer, BUFFLEN, in) == NULL)
		return -1;	/* EOF */

	ret = sscanf(buffer, "ID=%d %*s %*s %*s %*s Rx=%d.%d SIZE=%d\n", &id, &sec, &usec, &len);

	if (ret != 4)
		return 0;	/* skip|error */

	if (!mode)
		goto post_filter;

	if (flow_id && flow_id != id)
		return 0;	/* skip this flow */

post_filter:
	p->len = len + tare + DATALINK + IP + UDP;
	p->tv_sec = sec;
	p->tv_usec = usec;
	p->id = id;

	return 1;		/* OK */
}

/* --- Lindump parser --- */

int
parse_lindump(struct frame * p, FILE * in, int mode)
{
	char ftype;
        int sec, usec, len, arg_0, arg_1;
        int ret;

        if (fgets(buffer, BUFFLEN, in) == NULL)
                return -1;      /* EOF */

        ret = sscanf(buffer, "%c%*[:]%d.%d %d %d %x\n",&ftype,&sec, &usec, &len, &arg_0, &arg_1);

        if (ret < 4)
                return 0;       /* skip|error */


	if (!mode)
		goto post_filter;

	/* frame type filter */
	if ( f_filter && lut_filter[(unsigned)ftype] == 0 )
		return 0;	/* skip this frame */

	/* tos filter */
	if ( t_filter && ret==6 && lut_tos[arg_1&0xff] == 0 )
		return 0;

post_filter:
        p->len = len + tare + 4; /* 4 bytes = ethernet crc */
        p->tv_sec = sec;
        p->tv_usec = usec;

	switch(ret) {
		case 4:
			p->id = 0; 
			break;
		case 5: /* tstamp packlen ip_id */ 
			p->id = arg_0;
			break;
		case 6: /* tstamp packlen ip_id tos */ 
			p->id = arg_0; 
			break;
		default:
			return 0;
	}

        return 1;               /* OK */
}


/*
 * Return the difference of two timestamps in usec about
 * two datalink frames
 */
unsigned int
timestamp_diff(struct timeval * new, struct timeval * old)
{
        int msec;
        msec = (new->tv_sec - old->tv_sec) * 1000000 + (new->tv_usec - old->tv_usec);
        return msec > 0 ? msec: 0;
}

/* --- Sliding Window Algorithm --- */

/*
 * Alloc the frame, in case the distance from the last frame
 * is minor resolution; alloc an empty element otherwise
 */
struct frame *
alloc_frame(struct frame * frm)
{
	struct frame *ret;

	if (last != NULL && (timestamp_diff((struct timeval *)frm, (struct timeval *)last) > res )) {
		/* push a NULL frame */
		ret = (struct frame *) malloc(sizeof(struct frame));
		ret->tv_usec = (last->tv_usec + res) % 1000000;
		ret->tv_sec = last->tv_sec + (last->tv_usec + res) / 1000000;
		ret->len = 0;
		ret->id  = last_id; 
		ret->next = NULL;
	} else {
		/* push frame */
		ret = (struct frame *) malloc(sizeof(struct frame));
		ret->tv_sec = frm->tv_sec;
		ret->tv_usec = frm->tv_usec;
		ret->len = frm->len;
		ret->id = last_id = frm->id;
		ret->next = NULL;
	}
	return ret;
}

/*
 * Add the element/frame to list
 */
void
enqueue_frame(struct frame * n)
{
	/* enqueue the new element */
	if (last != NULL)
		last->next = n;
	last = n;

	if (head == NULL)	/* empty window */
		head = last;
}

/* --- Throughput --- */

/*
 * Slide the window removing frame lying out, and return
 * the sum of byte discarded or the number of frame discarded
 */
int
slide_window(int mode)
{
	struct frame *p;
	int ret = 0;

	while (head != NULL && timestamp_diff((struct timeval *)last, (struct timeval *)head) > winsize ) {
		switch (mode) {
		case SLIDE_BYTES:
			ret += head->len;
			break;
		case SLIDE_FRAMES:
		case SLIDE_LOSTP:
		case SLIDE_LOST:
			ret += (head->len > 0 ? 1 : 0);
			break;
		}
		p = head;
		head = head->next;
		free(p);
	}

	return ret;
}


int
sliding_mbps(struct frame * f, double *rate, int * sec, int * usec)
{
	static long long sum;
	struct frame *new;

	new = alloc_frame(f);
	enqueue_frame(new);

	/* add the current frame lenght to sum */
	sum += new->len;

	/* subtract the lenght of discarded frames */
	sum -= slide_window(SLIDE_BYTES);

	/* return values */
	*sec = new->tv_sec;
	*usec = new->tv_usec;
	*rate = ((double) ((sum << 3)) / ((double) winsize));

	return last->len;
}


int
sliding_fps(struct frame * f, double *rate, int * sec, int * usec)
{
	static long long sum;
	struct frame *new;

	new = alloc_frame(f);
	enqueue_frame(new);

	/* add the current frame to sum in case it is not empty */
	sum += (new->len ? 1 : 0);

	/* subtract the number of discarded frames */
	sum -= slide_window(SLIDE_FRAMES);

	/* return values */
	*sec = new->tv_sec;
	*usec = new->tv_usec;
	*rate = ((double) (sum * 1000000) / (double) winsize);

	return last->len;
}


/* --- Loss --- */

int
loss_fps(struct frame * f, double *rate, int * sec, int * usec)
{
        static long long sum;
	static double s_rate;
        struct frame *new;
        u_short delta_id;

        new = alloc_frame(f);
        enqueue_frame(new);

        /* add the current frame to sum in case it is lost */
        if (new->len != 0)
                sum++;

        /* subtract the number of discarded frames */
        sum -= slide_window(SLIDE_LOST);

        if (last == head) 
                delta_id = 0;
        else
                delta_id = (last->id-head->id);

        /* return values */
        *sec = new->tv_sec;
        *usec = new->tv_usec;

        if ( delta_id >= sum ) {
                s_rate = *rate = (double)((delta_id-sum)*1000000)/winsize;
        } else
                *rate = s_rate;

        return last->len;
}


int
loss_percent(struct frame * f, double *rate, int * sec, int * usec)
{
        static long long sum;
	static double s_rate;
        struct frame *new;
	u_short delta_id;

	/*
	 * sum = number of no NULL frame within the window
	 */

        new = alloc_frame(f);
        enqueue_frame(new);

        /* add the current frame to sum in case it is lost */
        if (new->len != 0)
                sum++;

        /* subtract the number of discarded frames */
        sum -= slide_window(SLIDE_LOSTP);

        if (last == head) 
		delta_id = 0;
        else	
		delta_id = (last->id-head->id);

        /* return values */
        *sec = new->tv_sec;
        *usec = new->tv_usec;

        if ( delta_id && delta_id >= sum ) 
                s_rate = *rate = (((double)delta_id-sum)*100)/(double)delta_id;
	else
                *rate = s_rate;

	return last->len;
}


#define PARSE_BUFFLEN	1024
#define PARSE_BUFFCOM	1024
static char parse_buff[PARSE_BUFFLEN];
static char parse_comm[PARSE_BUFFCOM];

void
brute_parse(char *file)
{
	FILE *f;
	int ret, msec, len, rate, dup = -1;
	int i = -1, flow_cnt = 0;
	char t_len;

	fprintf(stderr, "[r] brute: open(%s)\n", file);

	f = fopen(file, "r");
	if (f == NULL)
		fatal("can't open %s", file);

	for (;;) {
		if (flow_cnt == MAX_FLOWSTEP)
			break;

		if (fgets(parse_buff, PARSE_BUFFLEN, f) == NULL)	/* EOF */
			break;
		i++;
		msec = len = rate = dup = 0;

		ret = sscanf(parse_buff, "%79s %d %c:%d %d %*[x]%d",
			     parse_comm, &msec, &t_len, &len, &rate, &dup);

		/* --- skip: empty line or comment --- */

		if (ret == -1 || parse_comm[0] == '#' || parse_comm[1] == '#')
			continue;

		if (t_len != 'u' && t_len != 'f')
			fatal("brute_parse:%d unknown type '%c' lenght.",i+1,t_len);

		/* --- check the list of known command --- */
	
		/* command: flow */
		if (ret == 6 && strcmp("flow", parse_comm) == 0) {
			flows[flow_cnt].comm = COMM_FLOW;
			flows[flow_cnt].msec = ABS(msec);
			flows[flow_cnt].len = 0;
			flows[flow_cnt].rate = ABS(rate);
			flows[flow_cnt].tos = 0;
			flows[flow_cnt].ttl = 0;
			flows[flow_cnt].dup = dup;
			flow_cnt++;
			continue;
		}
		/* command: touch */
		if (ret == 6 && strcmp("touch", parse_comm) == 0) {
			flows[flow_cnt].comm = COMM_TOUCH;
			flows[flow_cnt].s_host = 0;
			flows[flow_cnt].d_host = 0;
			flows[flow_cnt].d_port = 0;
			flows[flow_cnt].msec = ABS(msec);
			flows[flow_cnt].len = 0;
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


/* based on the brute.data config file,
 * get_flowindex() returns the index of which the
 * frame belongs to
 */
int
get_flowindex(struct timeval *ts)
{
	long long rest; 
	int ret;
	rest = (long long)(ts->tv_sec) * 1000 + (long long)(ts->tv_usec) / 1000;
	for (ret=0; rest>0 && flows[ret].msec; ret++)
		rest -= (long long)flows[ret].msec;

	return ret-1;
}


void
display_options()
{

        fprintf(stderr, "[*] %s\n[+] processing %s data\n", cvsid,dumpfile[parse_opt]);
	
        /* Flow filters */
        if (flow_id && parse_opt == OPT_CRUDE)
                fprintf(stderr, "[-] processing flow ID=%d\n", flow_id);

        if (src_host && parse_opt == OPT_TCPDUMP)
                fprintf(stderr, "[-] source host.port {%s}\n", src_host);

        if (dst_host && parse_opt == OPT_TCPDUMP)
                fprintf(stderr, "[-] destination host.port {%s}\n", dst_host);

        if (t_filter) {
                int j;
                fprintf(stderr, "    tos: ");
                for (j=0;j<256;j++)
                        if (lut_tos[j]!=0)
                                fprintf(stderr,"|0x%.2x",j);
                fprintf(stderr,"\n");
        }

        fprintf(stderr, "[+] sliding window size: %d usec\n", winsize);
        fprintf(stderr, "    resolution: %d usec\n",res);
        fprintf(stderr, "[+] output type: --> (%s)\n", outtype[norm(output_opt)]);
}


void
dispatch_rate()
{
        struct timeval smp_in, smp_out;
        struct frame p;
        FILE *in, *out;
        int i, t, r, br_index;
	long long total;

	display_options();

        if (strcmp(filein, "stdin")) {
                /* open input file */
                fprintf(stderr, "[r] input : open(%s)\n", filein);
                if ((in = fopen(filein, "r")) == NULL)
                        fatal("can't open %s", filein);
        } else
                in = (FILE *) stdin;

        fprintf(stderr, "[w] output: open(%s)\n", fileout);
        if ((out = fopen(fileout, "w")) == NULL)
                fatal("can't open %s", fileout);

	fprintf(stderr, "    fdin=%d fdout=%d\n", fileno(in), fileno(out));

	brute_parse(filebrute);

	if (parse_opt & OPT_REALIGN)
        	for (i = 0, t = 0; (r = parser(&p, in,PARSE_FILTER)) == 0; t++)
                	continue;
	else
                for (i = 0, t = 0; (r = parser(&p, in,PARSE_NOFILTER)) == 0; t++)
                        continue;

        if (r == -1) {
                fprintf(stderr, "[+] processed %d/%d lines: done.\n", i, t);
                fclose(out);
                return;
        }

        /* set the origin */
        origin.tv_sec  = p.tv_sec;
        origin.tv_usec = p.tv_usec;
	br_index= 0;

	/* --- */

        for (i = 1; (r = parser(&p, in, PARSE_FILTER)) != -1; t++) {
                if (r == 0)
                        continue;

		smp_in.tv_sec = p.tv_sec;
		smp_in.tv_usec = p.tv_usec;
		timersub(&smp_in,&origin,&smp_out);
              
		/* get the flow index */
		br_index = get_flowindex(&smp_out);
		counter[br_index]++;
                i++;
        }

	for(br_index=0; flows[br_index].rate ; br_index++) {
		total = flows[br_index].rate*flows[br_index].msec/1000;

		if ( total < counter[br_index] )
			fprintf(out,"%d 0.0\n",flows[br_index].rate);
		else
			fprintf(out,"%d %f\n",flows[br_index].rate, 
			(double)(total-counter[br_index])/total);
	}

	/* --- */

        fprintf(stderr, "[+] processed %d/%d lines: done.\n", i, t);
        fclose(out);

        if (fileno(in) > 2)
                fclose(in);
}


void
dispatch_time()
{
	struct timeval smp_in, smp_out, smp_last;
	struct frame p;
	double rate;
	FILE *in, *out;
	int i, t, r;

	display_options();

	if (strcmp(filein, "stdin")) {
		/* open input file */
		fprintf(stderr, "[r] input : open(%s)\n", filein);
		if ((in = fopen(filein, "r")) == NULL)
			fatal("can't open %s", filein);
	} else
		in = (FILE *) stdin;

	fprintf(stderr, "[w] output: open(%s)\n", fileout);
	if ((out = fopen(fileout, "w")) == NULL)
		fatal("can't open %s", fileout);

	fprintf(stderr, "    fdin=%d fdout=%d\n", fileno(in), fileno(out));

	if (parse_opt & OPT_REALIGN)
		for (i = 0, t = 0; (r = parser(&p, in, PARSE_FILTER)) == 0; t++)
			continue;
	else
		for (i = 0, t = 0; (r = parser(&p, in, PARSE_NOFILTER)) == 0; t++)
                        continue;

	if (r == -1) {
		fprintf(stderr, "[+] processed %d/%d lines: done.\n", i, t);
		fclose(out);
		return;
	}
	
	/* set the origin */
	origin.tv_sec  = p.tv_sec;		
	origin.tv_usec = p.tv_usec;

	smp_last.tv_sec = 0;
	smp_last.tv_usec = 0;
	t++;

	for (i = 1; (r = parser(&p, in,PARSE_FILTER)) != -1; t++) {
		if (r == 0)
			continue;
		while (sliding_win(&p, &rate, (int *)&(smp_in.tv_sec), (int *)&(smp_in.tv_usec)) == 0) {
			/* dump rate for NULL frames */
			timersub(&smp_in,&origin,&smp_out);
			fprintf(out, "%d.%06d %f\n", (int)smp_out.tv_sec, (int)smp_out.tv_usec, rate);
		}

		/* dump the rate for the current frame */
		timersub(&smp_in,&origin,&smp_out);

		if ( timestamp_diff(&smp_out,&smp_last) > res ) {
			fprintf(out, "%d.%06d %f\n", (int)smp_out.tv_sec, (int)smp_out.tv_usec, rate);
			smp_last.tv_sec = smp_out.tv_sec;
			smp_last.tv_usec = smp_out.tv_usec;
		}
		i++;
	}

	fprintf(stderr, "[+] processed %d/%d lines: done.\n", i, t);
	fclose(out);

	if (fileno(in) > 2)
		fclose(in);
}


extern char *__progname;
void
usage()
{
	printf("%s [OPTIONS]:\n", __progname);
	printf(" source:\n");
	printf("   -i flow.in       specify the input file (default stdin)\n");
	printf("   -o flow.out      specify the output file (default flow.out)\n");
	printf("   -c               parse `crude` dump\n");
	printf("   -t               parse `tcpdump -ttv` dump\n");
	printf("   -x               parse `lindump` dump\n");
	printf(" output:\n");
	printf("   -b               throughput in Mbps (net)\n");
	printf("   -g               throughput in Mbps (gross)\n");
	printf("   -f               throughput in fps\n");
	printf("   -ldata           loss vs rate. (ip_id required, data is brute config file)\n");     
	printf("   -Lf              loss in fps.  (ip_id required)\n");
	printf("   -Lp              loss percent. (ip_id required)\n");
        printf(" lindump filter:\n");
        printf("   -Fh              PACKET_HOST\n");
        printf("   -Fb              PACKET_BROADCAST\n");
        printf("   -Fm              PACKET_MULTICAST\n");
        printf("   -Fp              PACKET_OTHERHOST\n");
        printf("   -Fo              PACKET_OUTGOING\n");
        printf("   -Fl              PACKET_LOOPBACK\n");
        printf("   -Ff              PACKET_FASTROUTE\n");
	printf("   -T 0xtos         select tos flow\n");
	printf(" tcpdump filter:\n");
        printf("   -S host.port     set source host.port\n");
        printf("   -D host.port     set destination host.port\n");
	printf(" crude filter:\n");
	printf("   -I id            select the flow ID\n");
	printf(" others:\n");
	printf("   -R usec          set the resolution (default 1000 usec)\n");
	printf("   -W msec          set the sliding window size (default 100 msec)\n");
	printf("   -w usec          set the sliding window size (usec)\n");
	printf("   -A               realign trace to origin\n");
	printf("   -v               print version\n");
	printf("   -h               print this help.\n");
	exit(1);
}


void
defaults()
{
	parse_opt = OPT_TCPDUMP;
	parser = parse_tcpdump;
	sliding_win = sliding_mbps;
	winsize = 100000;	/* usec */
	res     = 1000; 	/* usec */
	head = NULL;
	last = NULL;
}


int
main(int argc, char **argv)
{
	int i;

	defaults();

	while ((i = getopt(argc, argv, "hvtcxbgfT:o:i:w:W:I:S:D:l:L:R:A")) != EOF)
		switch (i) {
		case 'A':
			parse_opt = OPT_REALIGN;
			break;
		case 'T':
			t_filter = 1;
			lut_tos[strtol(optarg, (char **)NULL, 16)&0xff]=1;
			break;
                case 'F':
                        switch(optarg[0]) {
                        case 'h': f_filter |= OPT_FILTER_HOST; break;
                        case 'b': f_filter |= OPT_FILTER_BROADCAST; break;
                        case 'm': f_filter |= OPT_FILTER_MULTICAST; break;
                        case 'p': f_filter |= OPT_FILTER_OTHERHOST; break;
                        case 'o': f_filter |= OPT_FILTER_OUTGOING; break;
                        case 'l': f_filter |= OPT_FILTER_LOOPBACK; break;
                        case 'f': f_filter |= OPT_FILTER_FASTROUTE; break;
                        default:
                                fatal("unknown frame type");
                        }
                        break;
		case 't':
			parse_opt = OPT_TCPDUMP;
			parser = parse_tcpdump;
			break;
		case 'c':
			parse_opt = OPT_CRUDE;
			parser = parse_crude;
			break;
                case 'x':
                        parse_opt = OPT_LINDUMP;
                        parser = parse_lindump;
                        break;
		case 'b':
			output_opt |= OPT_BPS;
			sliding_win = sliding_mbps;
			break;
		case 'g':
			output_opt |= OPT_GROSS;
			sliding_win = sliding_mbps;
			tare = PREAMBLE+GAP; 
			break;
		case 'f':
			output_opt |= OPT_FPS;
			sliding_win = sliding_fps;
			break;
		case 'o':
			fileout = strdup(optarg);
			break;
		case 'i':
			filein = strdup(optarg);
			break;
		case 'l':
			output_opt |= OPT_LOSSRATE;
			filebrute = strdup(optarg);
			break;
		case 'L':
			switch(optarg[0]) {
			case 'f':
				optarg++;
        	                output_opt |= OPT_LOSS;
                	        sliding_win = loss_fps;
				break;
			case 'p':
				optarg++;
	                        output_opt |= OPT_LOSSP;
                	        sliding_win = loss_percent;
				break;
			default:
				fatal("unknown -L%s option\n",optarg);
			}
			break;
		case 'w':
			winsize = atoi(optarg);
			break;
		case 'W':
			winsize = atoi(optarg)*1000;
			break;
		case 'R':
			res = atoi(optarg);
			break;
		case 'I':
			flow_id = atoi(optarg);
			break;
		case 'S':
			src_host = strdup(optarg);
			break;
		case 'D':
			dst_host = strdup(optarg);
			break;
		case '?':
			fatal("opt: unknown option");
			break;
                case 'v':
                        fprintf(stderr, "%s\n", cvsid);
                        exit(1);
		case 'h':
			usage();
		}

	argc -= optind;
	argv += optind;

	if (output_opt == 0)
		output_opt |= OPT_BPS;

	/* lindump filter */

	if ( f_filter == 0 )
		f_filter = 0xffffffff;	/* no filter */ 

	switch (output_opt) {
	case OPT_BPS:
	case OPT_FPS:
	case OPT_GROSS:
	case OPT_LOSS:
	case OPT_LOSSP:
		dispatch_time();
		break;
	case OPT_LOSSRATE:
		dispatch_rate();
		break;
	default:
		fatal("opt: collision detected");
	}
	return 0;
}
