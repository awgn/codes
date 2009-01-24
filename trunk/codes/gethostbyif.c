/*
 *  $Id: gethostbyif.c,v 1.3 2003/05/01 16:52:17 awgn Exp $
 *  <ifbrowser>
 *
 *  Copyright (c) 1999 Bonelli Nicola <bonelli@antifork.org>
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

/*
Check if the system supports POSIX.1
*/

#if HAVE_UNISTD_H
#include <sys/types.h>
#include <unistd.h>
#endif

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

long
gethostbyif(char *ifname)
{
	char buffer[10240];
	int sd;
	struct ifreq *ifr, *iflast, ifreq_io;;
	struct ifconf ifc;
	struct sockaddr_in *ptr_if;

	memset(buffer, 0, 10240);

	/* dummy dgram socket for ioctl */

	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "%s!\n", strerror(errno));
		exit (-1);
	}
	ifc.ifc_len = sizeof(buffer);
	ifc.ifc_buf = buffer;

	/* getting ifs: this fills ifconf structure. */

	if (ioctl(sd, SIOCGIFCONF, &ifc) < 0) {
		fprintf(stderr, "%s\n", strerror(errno));
		exit (-1);
	}
	close(sd);

	/* line_up ifreq structure */

	ifr = (struct ifreq *) buffer;
	iflast = (struct ifreq *) ((char *) buffer + ifc.ifc_len);

	if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		fprintf(stderr, "%s!\n", strerror(errno));
		exit (-1);
	}
#if HAVE_SOCKADDR_SALEN
	for (; ifr < iflast;
	     (char *) ifr += sizeof(ifr->ifr_name) + ifr->ifr_addr.sa_len)
#else
	for (; ifr < iflast;
	 ifr += sizeof(ifr->ifr_name) + sizeof(struct sockaddr_in))
#endif
	{
		if (*(char *) ifr == 0)
			continue;

		ptr_if = (struct sockaddr_in *) & ifr->ifr_addr;
		memcpy(&ifreq_io, ifr, sizeof(ifr->ifr_name) + sizeof(struct sockaddr_in));

                if (ioctl(sd, SIOCGIFFLAGS, &ifreq_io) < 0) {
                        fprintf(stderr,"SIOCGIFFLAGS: %s\n", strerror(errno));
			exit(-1);
		}

                if ((ifreq_io.ifr_flags & IFF_UP) == 0)
                        continue;

                if (ifr->ifr_addr.sa_family != AF_INET)
                        continue;
		
		if (!strcmp(ifname, ifr->ifr_name)) {
			close(sd);
			return ptr_if->sin_addr.s_addr;
		}
	}

	close(sd);
	fprintf(stderr,"%s: interface not found\n",__FUNCTION__);
	return -1;
}

#ifdef IFBROWSER
extern char *__progname;
 
int
main(int argc, char **argv)
{	
        struct in_addr in;

	if ( argc < 2 ) {
		printf("usage: %s <ifname>\n",__progname);
		exit(1);
	}
	in.s_addr = gethostbyif(argv[1]);
	if ( in.s_addr == -1 )
		exit(2);
	printf("%s\n",inet_ntoa(in));

        return 0;
}
#endif
