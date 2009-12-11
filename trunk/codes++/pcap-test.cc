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

#include <pcap++.hh>
#include <tr1/functional>

class mycap : public more::pcap_live
{
public:
    mycap()
    : more::pcap_live("wlan0", 64, true, 0)
    {}

    virtual
    void packet_handler(const struct pcap_pkthdr *h, const u_char *p)
    {
        std::cout << *h << std::endl;
    }
};


// freefunction handler

void direct_handler(u_char *user, const struct pcap_pkthdr * h, const u_char *bytes)
{
    std::cout << "direct handler: " << *h << std::endl;
}

int
main(int argc, char *argv[])
{
    // std::string dev = more::pcap::lookupdev();
    // std::cout << "lookup device: " << dev << std::endl;

    mycap handle;
    
    std::cout << "device: " << handle.device() << std::endl;
    std::cout << "errbuf: [" << handle.errbuf() << "]" << std::endl;

    handle.nonblock(false);

    std::cout << "non-blocking mode: " << handle.nonblock() << std::endl;

    std::list<more::pcap_if> l = more::pcap::findalldevs();

    std::cout << "findalldevs: ";
    std::copy(l.begin(), l.end(), std::ostream_iterator<more::pcap_if>(std::cout, "\n             "));
    std::cout << std::endl;

    bpf_u_int32 net, mask;
    std::tr1::tie(net,mask) = more::pcap::lookupnet("lo");

    std::cout << "lookupnet: " << more::pcap::ipv4_dotform(net) << " " << more::pcap::ipv4_dotform(mask) << std::endl;
    std::cout << "dispatch: " << std::endl;

    std::cout << "datalink type:" << handle.datalink() << " len:" << handle.datalink_len() << " bytes" << std::endl;

    // for(;;) {
    //     handle.dispatch(10);
    // }

    {
        more::bpf_prog icmp_only("icmp");
        handle.filter(icmp_only);
    }

    // handle.loop(10);
    handle.loop(10, direct_handler, 0);  // use direct loop

    return 0;
}
 
