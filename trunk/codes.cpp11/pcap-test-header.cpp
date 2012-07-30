/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <iterator>
#include <algorithm>

#include <pcap++.hpp>
#include <network-headers.hpp>

using namespace more::net;

class mycap : public more::pcap::pcap_live
{
public:
    mycap()
    : more::pcap::pcap_live("eth0", 2048, true, 0)
    {}

    virtual
    void packet_handler(const struct pcap_pkthdr *h, const u_char *p)
    {
        auto cur = more::range_const_iterator(p, p + h->caplen);

        more::net::const_header<ethernet> eth_h(cur);
        more::net::const_header<ipv4> ip_h(cur);
        more::net::const_header<tcp> tcp_h(cur);

        std::cout << *h << "\n    " <<  *eth_h << 
                           "\n    " << *ip_h   << " csum_correct:" << std::boolalpha << ip_h->chksum_verify() << 
                           "\n    " <<  *tcp_h << " csum_correct:" << std::boolalpha << tcp_h->chksum_verify(*ip_h, cur.size()) << std::endl;

        // std::cout << *h << "\n    " <<  *eth_h << 
        //                   "\n    " << *ip_h   << " csum_correct:" << std::boolalpha << ip_h->check(net::verify()) << 
        //                   "\n    " <<  *icmp_h << std::endl;
    }
};


struct replay : public more::pcap::pcap_offline
{
    replay(const std::string &fname)
    : more::pcap::pcap_offline(fname)
    {}

    virtual
    void packet_handler(const struct pcap_pkthdr *h, const u_char *)
    {
        std::cout << "replay: " << *h << std::endl;
    }

};

// simple handler

void simple_handler(u_char *, const struct pcap_pkthdr * h, const u_char *)
{
    std::cout << "simple handler: " << *h << std::endl;
}


int
main(int, char *[])
{
    // std::string dev = more::pcap::lookupdev();
    // std::cout << "lookup device: " << dev << std::endl;

    mycap handle;
    
    std::cout << "device: " << handle.device() << std::endl;
    std::cout << "errbuf: [" << handle.errbuf() << "]" << std::endl;

    handle.nonblock(false);

    std::cout << "non-blocking mode: " << handle.is_nonblock() << std::endl;

    auto l = more::pcap::findalldevs();

    std::cout << "findalldevs: ";
    std::copy(l.begin(), l.end(), std::ostream_iterator<more::pcap::interface>(std::cout, "\n             "));
    std::cout << std::endl;

    bpf_u_int32 net, mask;
    std::tie(net,mask) = more::pcap::lookupnet("lo");

    std::cout << "lookupnet: " << more::pcap::ipv4_dotform(net) << " " << more::pcap::ipv4_dotform(mask) << std::endl;
    std::cout << "dispatch: " << std::endl;

    std::cout << "datalink type:" << handle.datalink() << " len:" << handle.datalink_len() << " bytes" << std::endl;

    // for(;;) {
    //     handle.dispatch(10);
    // }

    {
        more::pcap::bpf_prog icmp_only("tcp");
        handle.filter(icmp_only);
    }

    std::cout << "dumping 5 tcp-segments to cout..." << std::endl;

    handle.loop(5);        
 
    std::cout << "dumping 5 tcp-segments to file..." << std::endl;
    {
        more::pcap::pcap_dumper test(handle, "test.pcap");
        handle.loop(5, more::pcap::pcap_dumper::handler, reinterpret_cast<u_char *>(&test) );  // use direct loop
        test.flush();
    }

    std::cout << "dumping 5 tcp-segments from file..." << std::endl;
    {
        replay r("test.pcap");
        r.loop(5);
    }

    return 0;
}
 
