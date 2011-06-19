/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <network-headers.hpp>
#include <iostream>

using namespace more::net;

int
main(int argc, char *argv[])
{
    std::cout << "write test..." << std::boolalpha << std::endl;

    // write test...
    {
        char buf[68]={ '\0' };

        auto cur = more::range_iterator(buf, buf+68);

        more::net::header<ethernet> h_eth(cur);
        more::net::header<eth802_1q> h_vlan(cur);

        h_eth->dhost("0:1:2:3:4:5");
        h_eth->shost("a:b:c:d:e:f");
        h_eth->ether_type(ethernet::type_8021q);

        h_vlan->vlan_tag(0xbabe);
        h_vlan->ether_type(0x800);

        std::cout << "eth: " << h_eth->size() << " bytes " << *h_eth << std::endl;
        std::cout << "vlan: " << *h_vlan << std::endl;

        more::net::header<ipv4> h_ip(cur,20);

        h_ip->version(4);

        h_ip->saddr("192.168.0.1");
        h_ip->daddr("192.168.0.100");
        h_ip->tot_len(50);

        h_ip->chksum_update();

        std::cout << " ip: " << h_ip->size() << " bytes " << *h_ip << std::endl;
        std::cout << "ip_checksum: " << h_ip->chksum_verify() << std::endl;

        more::net::header<tcp> h_tcp(cur,20);

        h_tcp->source(1024);
        h_tcp->dest(31337);
        h_tcp->check(0);

        h_tcp->flags_reset();

        h_tcp->ece(true);
        h_tcp->cwr(true);
        h_tcp->syn(true);

        h_tcp->chksum_update(*h_ip, cur.capacity() /* bytes of tcp segment */);

        std::cout << "tcp: " << h_tcp->size() << " bytes " << *h_tcp << std::endl;

        std::cout << "tcp_checksum: " << h_tcp->chksum_verify(*h_ip, cur.capacity()) << std::endl;  
        std::cout << "payload: " << cur.capacity() << " bytes" << ", offset: " << cur.size() << std::endl;
    }

    std::cout << "read test..." << std::endl;

    // read test
    {
        char buf[54] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x45, 0x45, 0x45};

        auto cur = more::range_const_iterator(buf, buf+54);

        more::net::const_header<ethernet> h_eth(cur);
        more::net::const_header<ipv4>     h_ip(cur);
        more::net::const_header<tcp>      h_tcp(cur);
    }

}
