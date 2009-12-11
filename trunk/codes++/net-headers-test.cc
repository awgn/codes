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
    char buf[16];
    {
        std::cout << "ethernet: " << net::ethernet::size() << " bytes" << std::endl;

        more::header<net::ethernet> h(buf);

        h->dhost("0:1:2:3:4:5");
        h->shost("a:b:c:d:e:f");
        h->ether_type(0x800);

        std::cout << *h << std::endl;
    }
}
