/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <net-headers.hh>
#include <iostream>

int
main(int argc, char *argv[])
{
    char buf[80] = "12alkjhflahdflakshfakldfhalkhdfalkdfhalkhdfalkdhfalhfd";

    ssize_t len = 80;
    char * p = buf;

    {
        more::header<net::ethernet> h(p,len);

        h->dhost("0:1:2:3:4:5");
        h->shost("a:b:c:d:e:f");
        h->ether_type(0x800);

        std::cout << "eth: " << h->size() << " bytes " << *h << std::endl;
    }

    {
        more::header<net::ipv4> h(p,len);

        h->ihl(5);
        h->version(4);

        h->saddr("192.168.0.1");
        h->daddr("192.168.0.100");
        h->check(net::update());

        std::cout << " ip: " << h->size() << " bytes " << *h << std::endl;
        // std::cout << "checksum: " << h->check(net::verify()) << std::endl;
    }

    {
        more::header<net::udp> h(p,len);

        h->source(1024);
        h->dest(31337);
        h->len(64);
        h->check(0);

        std::cout << "udp:  " << h->size() << " bytes " << *h << std::endl;
    }

}
