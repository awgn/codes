/*
 *  $Id: if.c,v 1.5 2004/07/10 13:04:29 awgn Exp $
 *  <ifbrowser>
 *
 *  Copyright (c) 1999 Bonelli Nicola <awgn@antifork.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  In order to compile this routine properly you have to include
 *  the following macros in configure.in:
 *
 *  AC_HEADER_STDC
 *
 *  AC_CACHE_CHECK(if sockaddr has sa_len member, ac_cv_sockaddr_has_sa_len,
 *         AC_TRY_COMPILE([
 *  #               include <sys/types.h>
 *  #               include <sys/socket.h>],
 *                  [unsigned int i = sizeof(((struct sockaddr *)0)->sa_len)],
 *          ac_cv_sockaddr_has_sa_len=yes,
 *          ac_cv_sockaddr_has_sa_len=no))
 *
 *  Otherwise compile a test with the command line below:
 *
 *  (note: HAVE_SOCKADDR_SALEN must be defined whenever the sockaddr has
 *         the salen member or not. Linux, unlike *BSD, doesn't need it. )
 *  LINUX:
 *	    gcc -DTEST_IFBROWSER if.c -o ifb
 *  *BSD:
 *          gcc -DTEST_IFBROWSER -DHAVE_SOCKADDR_SALEN if.c -o ifb
 *
 */

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <sysexits.h>

const char usage_s[] = "\
Usage:%s [options]                                               \n\
   -v                   display the version and exit.            \n\
   -h                   print this help.                         \n";

#ifndef __unused
#define __unused        __attribute__((unused))
#endif
#ifndef __dead
#define __dead          __attribute__((noreturn))
#endif


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


/* usage */
__dead void
usage()
{
        extern char *__progname;
        printf(usage_s, __progname);
        exit(0);
}



static char if_buffer[10240];

void
if_browser()
{
	struct ifreq *ifr, *iflast, ifreq_io;
	struct ifconf ifc;
	struct sockaddr_in *ptr;
	int sd;


	/* dummy dgram socket for ioctl */
	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
		fatal("socket()");

	ifc.ifc_buf = if_buffer;
	ifc.ifc_len = sizeof(if_buffer);

	/* getting ifs: this fills ifconf structure. */
	if (ioctl(sd, SIOCGIFCONF, &ifc) < 0) 
		fatal("SIOCGIFCONF");

	close(sd);

	/* line_up ifreq structure */
	ifr    = (struct ifreq *) if_buffer;
	iflast = (struct ifreq *) ((char *) if_buffer + ifc.ifc_len);

	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) 
		fatal("socket()");

#if HAVE_SOCKADDR_SALEN
	for (; ifr < iflast;
	     	ifr = (struct ifreq *)( (void *)ifr+ sizeof(ifr->ifr_name)+ ifr->ifr_addr.sa_len) )
#else
	for (; ifr < iflast;
	 	ifr = (struct ifreq *)( (void *)ifr+ sizeof(ifr->ifr_name)+ sizeof(struct sockaddr_in) ) )
#endif
	{
		ptr = (struct sockaddr_in *) & ifr->ifr_addr;
		memcpy(&ifreq_io, ifr, sizeof(ifr->ifr_name) + sizeof(struct sockaddr_in));

		if (ioctl(sd, SIOCGIFFLAGS, &ifreq_io) < 0) {
			fprintf(stderr, "SIOCGIFFLAGS: %s", strerror(errno));
			exit(1);		
		}

		/* filters: IFF_UP & AF_INET */
//		if ((ifreq_io.ifr_flags & IFF_UP) == 0)
//			continue;

//		if (ifr->ifr_addr.sa_family != AF_INET && 
//		    ifr->ifr_addr.sa_family != AF_INET6 )
//			continue;

		printf("%s\tinet addr:%s ", ifr->ifr_name, inet_ntoa(ptr->sin_addr));

		/* SIOCGIFNETMASK */
		memcpy(&ifreq_io, ifr, sizeof(ifr->ifr_name) + sizeof(struct sockaddr_in));
		if (ioctl(sd, SIOCGIFNETMASK, &ifreq_io) < 0) {
			fprintf(stderr, "SIOCGIFNETMASK:%s\n", strerror(errno));
			exit(2);		
		} 

		ptr = (struct sockaddr_in *) & ifreq_io.ifr_addr;
		printf("mask: %s\n", inet_ntoa(ptr->sin_addr));

		/* SIOCGIFMAP */
		memcpy(&ifreq_io, ifr, sizeof(ifr->ifr_name) + sizeof(struct sockaddr_in));
                if (ioctl(sd, SIOCGIFMAP, &ifreq_io) < 0) {
                        fprintf(stderr, "SIOCGIFMEM:%s\n", strerror(errno));
                        exit(2);                
                }

		printf("\tbase_addr:%x memory:%x-%x irq=%d dma=%d port=%d\n", 
			(unsigned int)ifreq_io.ifr_ifru.ifru_map.base_addr,
			(unsigned int)ifreq_io.ifr_ifru.ifru_map.mem_start,
			(unsigned int)ifreq_io.ifr_ifru.ifru_map.mem_end,
			(unsigned int)ifreq_io.ifr_ifru.ifru_map.irq,
			(unsigned int)ifreq_io.ifr_ifru.ifru_map.dma,
			(unsigned int)ifreq_io.ifr_ifru.ifru_map.port);	

		putchar('\n');
	}
	close(sd);
	return;
}

int
main(argc,argv)
	int argc;
	char **argv;
{
	if_browser();
	return 0;
}
