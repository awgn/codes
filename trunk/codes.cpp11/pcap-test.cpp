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

class mycap : public more::pcap::pcap_live
{
public:
    mycap()
    : more::pcap::pcap_live("eth0", 64, true, 0)
    {}

    mycap(mycap &&other)
    : more::pcap::pcap_live(std::move(other))
    {}

    mycap& operator=(mycap &&other)
    {
        if (this != &other)
        {
            static_cast<more::pcap::pcap_live *>(this)->operator=(std::move(other));
        }
        return *this;
    }

    virtual
    void packet_handler(const struct pcap_pkthdr *h, const u_char *)
    {
        std::cout << *h << std::endl;
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
    // test for moveability...
    //

    std::vector<mycap> handle;

    handle.push_back(mycap());

    std::cout << "device: " << handle[0].device() << std::endl;
    std::cout << "errbuf: [" << handle[0].errbuf() << "]" << std::endl;

    handle[0].nonblock(false);

    std::cout << "non-blocking mode: " << handle[0].is_nonblock() << std::endl;

    std::vector<more::pcap::interface> l = more::pcap::findalldevs();

    std::cout << "findalldevs: ";
    std::copy(l.begin(), l.end(), std::ostream_iterator<more::pcap::interface>(std::cout, "\n             "));
    std::cout << std::endl;

    bpf_u_int32 net, mask;
    std::tie(net,mask) = more::pcap::lookupnet("lo");

    std::cout << "lookupnet: " << more::pcap::ipv4_dotform(net) << " " << more::pcap::ipv4_dotform(mask) << std::endl;
    std::cout << "dispatch: " << std::endl;

    std::cout << "datalink type:" << handle[0].datalink() << " len:" << handle[0].datalink_len() << " bytes" << std::endl;

    // for(;;) {
    //     handle[0].dispatch(10);
    // }

    {
        more::pcap::bpf_prog icmp_only("icmp");
        handle[0].filter(icmp_only);
    }

    std::cout << "dumping 10 icmp to cout..." << std::endl;

    handle[0].loop(10);
    // handle[0].loop(10, simple_handler);  // use direct loop

    std::cout << "dumping 10 icmp to file..." << std::endl;
    {
        more::pcap::pcap_dumper test(handle[0], "test.pcap");
        handle[0].loop(10, more::pcap::pcap_dumper::handler, reinterpret_cast<u_char *>(&test) );  // use direct loop
        test.flush();
    }

    std::cout << "dumping 10 icmp from file..." << std::endl;
    {
        replay r("test.pcap");
        r.loop(10);
    }

    return 0;
}

