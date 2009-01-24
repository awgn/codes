/*
 * $Id: ei.c,v 1.3 2003/10/25 13:56:01 awgn Exp $
 *
 * Copyright (c) 2003 Bonelli Nicola <bonelli@antifork.org>
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


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#define __KERNEL__
#include <asm/types.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


struct ethtool_drvinfo *
ethernet_info(const char *ifname)
{
	static struct ethtool_drvinfo *info;
	struct ifreq ifr;
	int s;
	u32 req = ETHTOOL_GDRVINFO;	/* netdev ethcmd */

	s = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (s == -1) {
		fprintf(stderr, "socket(): %s\n", strerror(errno));
		return NULL;
	}
	strcpy(ifr.ifr_name, ifname);

	free(info);
        ifr.ifr_data = (__caddr_t) malloc(sizeof(struct ethtool_drvinfo));
	info = (struct ethtool_drvinfo *) ifr.ifr_data;

	if (ifr.ifr_data == NULL) {
		fprintf(stderr, "malloc(): %s\n", strerror(errno));
		return NULL;
	}
	strncpy(ifr.ifr_data, (char *) &req, sizeof(req));

	if (ioctl(s, SIOCETHTOOL, &ifr) == -1) {
		fprintf(stderr, "warning: the ethernet netdrv don't support SIOCETHTOOL ioctl()!\n");
		return NULL;
	}

	return info;
}

int
main(argc, argv)
	int argc;
	char **argv;
{
	struct ethtool_drvinfo *p;
	p = ethernet_info("eth0");
        if ( p == NULL )
                printf("err\n"), exit(1);
               
        printf("%s\n", p->driver);
        printf("%s\n", p->version);
        printf("%s\n", p->fw_version);

        return 0;
}
