/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <net-headers.hpp>
#include <iostream>

using namespace more::net;

  int
main(int argc, char *argv[])
{
    std::cout << "write test..." << std::boolalpha << std::endl;

    // write test...
    {
        char buf[68]={ '\0' };

        more::cursor<char> cur(buf, buf+68);

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

        h_ip->check_update();

        std::cout << " ip: " << h_ip->size() << " bytes " << *h_ip << std::endl;
        std::cout << "ip_checksum: " << h_ip->check_verify() << std::endl;

        // {
        //     more::net::header<udp> h(cur);
        //     h->source(1024);
        //     h->dest(31337);
        //     h->len(64);
        //     h->check(0);
        //     std::cout << "udp:  " << h->size() << " bytes " << *h << std::endl;
        // }

        more::net::header<tcp> h_tcp(cur,20);

        h_tcp->source(1024);
        h_tcp->dest(31337);
        h_tcp->check(0);

        h_tcp->flags_reset();

        h_tcp->ece(true);
        h_tcp->cwr(true);
        h_tcp->syn(true);

        h_tcp->check_update(*h_ip, cur.capacity() /* bytes of tcp segment */);

        std::cout << "tcp: " << h_tcp->size() << " bytes " << *h_tcp << std::endl;

        std::cout << "tcp_checksum: " << h_tcp->check_verify(*h_ip, cur.capacity()) << std::endl;  
        std::cout << "payload: " << cur.capacity() << " bytes" << ", offset: " << (cur.cur()-cur.begin()) << std::endl;

        // std::cout << "eth_min_size: " << ethernet::__min_size << std::endl;
        // std::cout << " ip_min_size: " << ipv4::__min_size << std::endl;
        // std::cout << "tcp_min_size: " << tcp::__min_size << std::endl;
    }

    std::cout << "read test..." << std::endl;

    // read test, truncated header
    {
        char buf[35] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x45, 0x45, 0x45};

        more::cursor<char> cur(buf, buf+54);

        more::net::header<ethernet> h_eth(cur);
        more::net::header<ipv4>     h_ip(cur);
        more::net::header<tcp>      h_tcp(cur);

    }

}
