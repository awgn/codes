/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <netdev.hpp>
#include <iterator>
#include <algorithm>

struct dump
{
    dump(std::ostream &out)
    : m_out(out)
    {}

    void operator()(const more::netdev::ifr &i)
    {
        m_out << i.name() << ' ';
    }

    std::ostream &m_out;
};

  int
main(int, char *[])
{
    more::netdev::ifr dev("eth0");

    auto devs = more::netdev::ifr::enumerate();

    std::cout << "enum        : ";
    std::for_each(devs.begin(), devs.end(), dump(std::cout));
    std::cout << std::endl;

    std::cout << "receive_st  : " << dev.receive_stat() << std::endl;
    std::cout << "transmit_st : " << dev.transmit_stat() << std::endl;
    std::cout << "flags.val   : " << dev.flags().value() << std::endl;
    std::cout << "flags.str   : " << dev.flags().str() << std::endl;

    std::cout << "ip-addr     : " << dev.if_addr() << std::endl;
    std::cout << "ip-destaddr : " << dev.if_dstaddr() << std::endl;
    std::cout << "netmask     : " << dev.netmask() << std::endl;
    std::cout << "broadcast   : " << dev.broadcast() << std::endl;
    std::cout << "mac         : " << dev.hwaddr() << std::endl;
    std::cout << "mtu         : " << dev.mtu() << std::endl;
    std::cout << "metric      : " << dev.metric() << std::endl;
    std::cout << "index       : " << dev.index() << std::endl;
    std::cout << "qlen        : " << dev.qlen() << std::endl;

    std::cout << "is_up       : " << dev.is_up() << std::endl;
    std::cout << "is_broadcast: " << dev.is_broadcast() << std::endl;
    std::cout << "is_debug    : " << dev.is_debug() << std::endl;
    std::cout << "is_loopback : " << dev.is_loopback() << std::endl;
    std::cout << "is_ptp      : " << dev.is_ptp() << std::endl;
    std::cout << "is_notrl    : " << dev.is_notrl() << std::endl;
    std::cout << "is_running  : " << dev.is_running() << std::endl;
    std::cout << "is_noarp    : " << dev.is_noarp() << std::endl;
    std::cout << "is_promisc  : " << dev.is_promisc() << std::endl;
    std::cout << "is_allmulti : " << dev.is_allmulti() << std::endl;
    std::cout << "is_master   : " << dev.is_master() << std::endl;
    std::cout << "is_slave    : " << dev.is_slave() << std::endl;
    std::cout << "is_multicast: " << dev.is_multicast() << std::endl;
    std::cout << "is_portsel  : " << dev.is_portsel() << std::endl;
    std::cout << "is_automedia: " << dev.is_automedia() << std::endl;

    auto drvinfo = dev.drvinfo();
    
    std::cout << "driver      : " << drvinfo->driver << std::endl;
    std::cout << "version     : " << drvinfo->version << std::endl;
    std::cout << "bus_info    : " << drvinfo->bus_info << std::endl;

    auto cmd = dev.command();

    std::cout << "speed       : " << cmd->speed << std::endl;
    std::cout << "link        : " << dev.link() << std::endl;
    
   return 0;
}
 
