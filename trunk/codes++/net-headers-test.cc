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

    more::cursor<char> cur(buf, buf+64);
    
    more::header<net::ethernet> h_eth(cur);

    h_eth->dhost("0:1:2:3:4:5");
    h_eth->shost("a:b:c:d:e:f");
    h_eth->ether_type(0x800);

    std::cout << "eth: " << h_eth->size() << " bytes " << *h_eth << std::endl;
   

    more::header<net::ipv4> h_ip(cur,20);

    h_ip->version(4);

    h_ip->saddr("192.168.0.1");
    h_ip->daddr("192.168.0.100");
    h_ip->check_update();

    h_ip->tot_len(50);

    std::cout << " ip: " << h_ip->size() << " bytes " << *h_ip << std::endl;
    std::cout << "ip_checksum: " << h_ip->check_verify() << std::endl;
    
    // {
    //     more::header<net::udp> h(cur);

    //     h->source(1024);
    //     h->dest(31337);
    //     h->len(64);
    //     h->check(0);

    //     std::cout << "udp:  " << h->size() << " bytes " << *h << std::endl;
    // }


    more::header<net::tcp> h_tcp(cur,20);

    h_tcp->source(1024);
    h_tcp->dest(31337);
    h_tcp->check(0);

    h_tcp->flags_reset();

    h_tcp->ece(true);
    h_tcp->cwr(true);
    h_tcp->syn(true);

    h_tcp->check_update(*h_ip, cur.size() /* bytes of tcp segment */);

    std::cout << "tcp: " << h_tcp->size() << " bytes " << *h_tcp << std::endl;

    std::cout << "tcp_checksum: " << h_tcp->check_verify(*h_ip, cur.size()) << std::endl;  
    std::cout << "payload: " << cur.size() << " bytes" << ", offset: " << (cur.cur()-cur.begin()) << std::endl;
}
