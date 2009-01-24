/*
 * *
 *  *  pcap2ixp:
 *   * convert raw pcap dump into traffic.dll format for SDK 2.01 WorkBench 
 *    *
 *     * Copyright (c) 2003 Nicola Bonelli <bonelli@antifork.org>
 *      *
 *       * All rights reserved.
 *        *
 *         * Tested on some linux and BSD flavors
 *          *
 *           * Redistribution and use in source and binary forms, with or without
 *            * modification, are permitted provided that the following conditions are
 *             * met: 1. Redistributions of source code must retain the above copyright
 *              * notice, this list of conditions and the following disclaimer. 2.
 *               * Redistributions in binary form must reproduce the above copyright notice,
 *                * this list of conditions and the following disclaimer in the documentation
 *                 * and/or other materials provided with the distribution.
 *                  *
 *                   * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND ANY
 *                    * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *                     * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *                      * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE FOR
 *                       * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *                        * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *                         * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *                          * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *                           * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *                            * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 *                             * SUCH DAMAGE.
 *                              *
 *                               *
 *                                */

#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/uio.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>

static char cvsid[] = "$Id: pcap2ixp.c,v 1.5 2003/10/25 13:56:01 awgn Exp $";

/* 
 * typedef 
 */

typedef int bpf_int32;
typedef u_int bpf_u_int32;


/********** 
 * pcap.h * 
 **********/

struct pcap_file_header {
	bpf_u_int32 magic;
	u_short version_major;
	u_short version_minor;
	bpf_int32 thiszone;     /* gmt to local correction */
	bpf_u_int32 sigfigs;    /* accuracy of timestamps */
	bpf_u_int32 snaplen;    /* max length saved portion of each pkt */
	bpf_u_int32 linktype;   /* data link type (LINKTYPE_*) */
};

/*
 *  Each packet in the dump file is prepended with this generic header.
 *  This gets around the problem of different headers for different
 *  packet interfaces.
 */

struct pcap_timeval {
	bpf_int32 tv_sec;           /* seconds */
	bpf_int32 tv_usec;          /* microseconds */
};

struct pcap_sf_pkthdr {
    	struct pcap_timeval ts;     /* time stamp */
    	bpf_u_int32 caplen;         /* length of portion present */
	bpf_u_int32 len;            /* length this packet (off wire) */
};

/*
 * Macro
 */

#define MACHDR	14
#define IPHDR	20

/*
 * Global variables
 */

extern char *__progname;
char *filein;
char *fileout;

int n_packet;

/* 
 * Usage string 
 */

static char usage_str[]="\
%s [OPTIONS]:                                           \n\
   -i file          input pcap file                     \n\
   -o file          output ixp file                     \n\
   -n packet        number of packet                    \n\
 others:                                                \n\
   -v               print version                       \n\
   -h               print this help                     \n";


/*
 * Fatal
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

#define BUFFLEN	1500

struct pcap_sf_pkthdr pcap_pkthdr;
unsigned char pcap_packet[BUFFLEN];

/*
 * Return the difference of two timestamps in usec 
 */

unsigned int
timestamp_diff(struct timeval * new, struct timeval * old)
{
        int msec;
	msec = (new->tv_sec - old->tv_sec) * 1000000 + (new->tv_usec - old->tv_usec);
	return msec > 0 ? msec: 0;
}
	

void
dump_machdr(FILE *file,unsigned char *p)
{
  	int i;
  	for(i=0;i<MACHDR;i++)
  		fprintf(file,"%.2x ",p[i]);
  	fprintf(file,"\n");
}


void
dump_iphdr(FILE *file, unsigned char *p)
{
 	int i;
        for(i=MACHDR;i<(MACHDR+IPHDR);i++) {
		fprintf(file,"%.2x ",p[i]);
		if ( i == (MACHDR+9) )	
			fprintf(file,"\n");
	}
	fprintf(file,"\n");
}


void
dump_payload(FILE *file, unsigned char *p,int len, int caplen)
{
	int i,j;
	
	/* dump the playload until it's available, fill with 0xff subsequently */
	
	for (i=(MACHDR+IPHDR),j=0; i < len ; i++,j++ ) {
		if (i < caplen)
			fprintf(file,"%.2x ", p[i]);
		else
			fprintf(file,"ff ");

		if (j==9) {
			fprintf(file,"\n");
			j=-1;
		}	
	}
	fprintf(file,"\n");
}


void
convert()
{
	struct pcap_file_header hdr;
	struct timeval now;
	int fd_in, i,b,intertime;
	FILE *FD_OUT;

	if ( (fd_in=open(filein,O_RDONLY)) == -1)
                fatal("%s open() error",filein);

	if ( (FD_OUT=fopen(fileout,"w"))== NULL )
		fatal("%s fopen() error",fileout);

	if ( read(fd_in,(void *)&hdr,sizeof(hdr)) != sizeof(hdr) )
		fatal("%s error: file truncated",filein);

#define DUMP(x) fprintf(stderr,"[i] %s = 0x%x\n",#x,x)
	DUMP(hdr.magic);
	DUMP(hdr.version_major);
	DUMP(hdr.version_minor);
	DUMP(hdr.thiszone);
	DUMP(hdr.snaplen);
	DUMP(hdr.sigfigs);
	DUMP(hdr.linktype);

	fprintf(FD_OUT,"%d\n",n_packet);
	gettimeofday(&now,NULL);

	for (i=0;i<n_packet;i++) {
		if ( (b=read(fd_in,&pcap_pkthdr,sizeof(struct pcap_sf_pkthdr))) 
			!= sizeof(struct pcap_sf_pkthdr))
			break;
			
		if ( (b=read(fd_in,pcap_packet,pcap_pkthdr.caplen)) <=0 )
			break;

		fprintf(stderr,"    %d:{%d.%.6d} caplen=%d framelen=%d\n",i+1,
			pcap_pkthdr.ts.tv_sec,pcap_pkthdr.ts.tv_usec, pcap_pkthdr.caplen, pcap_pkthdr.len);

		/* inter-time */
		intertime = timestamp_diff((struct timeval *)&pcap_pkthdr.ts,&now);
		now.tv_sec  = pcap_pkthdr.ts.tv_sec;
		now.tv_usec = pcap_pkthdr.ts.tv_usec; 

		fprintf(FD_OUT,"%d\n",intertime*1000);

		/* packet len */
		fprintf(FD_OUT,"%d\n",pcap_pkthdr.len);

		/* headers */
		dump_machdr(FD_OUT,pcap_packet);
		dump_iphdr(FD_OUT,pcap_packet);
		dump_payload(FD_OUT,pcap_packet,pcap_pkthdr.len,pcap_pkthdr.caplen);
	}	

	fprintf(stderr,"[*] done.\n");
}


int
main(argc, argv)
	int argc;
	char ** argv;
{
	int i;

	while ((i=getopt(argc,argv,"i:o:n:vh")) != EOF )
		switch(i) {
			case 'i':
				filein = optarg;
				break;
			case 'o':
				fileout = optarg;
				break;
			case 'n':
				n_packet = atoi(optarg);
				break;
			case 'v':
				fprintf(stderr, "%s\n", cvsid);
				exit(1);
			case 'h':
				printf(usage_str,__progname);
				exit(1);        
			case '?':
			default:
				fatal("opt: unknown option");
				break;
		}
	
	argc -= optind;
	argv += optind;

	/* --- */
	
	if ( filein == NULL )
		fatal("input file not given");

	if ( fileout == NULL )
		fatal("output file not given");

	fprintf(stderr, "[*] %s: %s\n", __progname, cvsid);
	convert();
	
	return 0;
}	
