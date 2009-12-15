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
    char buf[64]={ '\0' };

    ssize_t len = 64;
    char * p = buf;

    std::cout << "len-> " << len << " bytes" << std::endl;

    {
        more::header<net::ethernet> h(p,len);

        h->dhost("0:1:2:3:4:5");
        h->shost("a:b:c:d:e:f");
        h->ether_type(0x800);

        std::cout << "eth: " << h->size() << " bytes " << *h << std::endl;
    }

    {
        more::header<net::ipv4> h(p,len,20);

        h->version(4);

        h->saddr("192.168.0.1");
        h->daddr("192.168.0.100");
        h->check(net::update());

        std::cout << " ip: " << h->size() << " bytes " << *h << std::endl;
        std::cout << "checksum: " << h->check(net::verify()) << std::endl;
    }
    
    // {
    //     more::header<net::udp> h(p,len);

    //     h->source(1024);
    //     h->dest(31337);
    //     h->len(64);
    //     h->check(0);

    //     std::cout << "udp:  " << h->size() << " bytes " << *h << std::endl;
    // }


    {
        more::header<net::tcp> h(p,len,20);

        h->source(1024);
        h->dest(31337);
        h->check(0);

        h->flags_reset();

        h->ece(true);
        h->cwr(true);
        h->syn(true);

        std::cout << "tcp: " << h->size() << " bytes " << *h << std::endl;
    }

    std::cout << "payload: " << len << " bytes" << ", offset: " << (p-buf) << std::endl;
}
