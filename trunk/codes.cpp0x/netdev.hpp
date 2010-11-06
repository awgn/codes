/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _NETDEV_HPP_
#define _NETDEV_HPP_ 

#include <token.hpp>        // more!

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <linux/ethtool.h>
#include <linux/version.h>
#include <linux/sockios.h>
#include <asm/types.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <cassert>
#include <string>
#include <cstring>
#include <limits>
#include <cctype>
#include <memory>
#include <vector>
#include <stdexcept>

namespace more { namespace netdev
{
    struct isalnum_t
    {
        bool operator()(char c)
        {
            return std::isalnum(c);
        }
    };
    typedef more::basic_token<isalnum_t> devname_t;

    /// stats per dev...

    struct stat
    {
        int bytes;
        int packets;
        int errs;
        int drop;
        int fifo;
        int frame;
        int compressed;
        int multicast;
    };

    template <typename CharT, typename Traits>
    inline typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits> &in, stat &rhs)
    {
        return in >> rhs.bytes >> rhs.packets >> rhs.errs >> rhs.drop >> rhs.fifo >>
               rhs.frame >> rhs.compressed >> rhs.multicast;
    }

    template <typename CharT, typename Traits>
    inline typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const stat &rhs)
    {
        return out << rhs.bytes << ' ' << rhs.packets << ' ' << rhs.errs << ' ' 
                << rhs.drop << ' ' << rhs.fifo << ' ' 
                 << rhs.frame << ' ' << rhs.compressed << ' ' << rhs.multicast;
    }

    ////////////////////////////////////
    /// get information of a net device 

    class ifr
    {
    public:
        ifr(const char * dev)
        : _M_ifreq_io(), _M_dev(dev), _M_sock(::socket(AF_INET, SOCK_DGRAM,0))
        {
            strncpy(_M_ifreq_io.ifr_name, dev, IFNAMSIZ);
            if (_M_sock == -1)
                throw std::runtime_error("socket");
        }        
        
        ~ifr()
        {}

        ///

        std::string
        name() const
        {
            return _M_dev;
        }

        static std::vector<std::string>
        enumerate() 
        {
            std::vector<std::string> vec;
            std::ifstream proc("/proc/net/dev"); assert(proc);

            // skip the first couple of lines:
            proc.ignore(std::numeric_limits<std::streamsize>::max(), proc.widen('\n'));
            proc.ignore(std::numeric_limits<std::streamsize>::max(), proc.widen('\n'));
            
            devname_t dev;
            while(proc >> dev)
            {
                vec.push_back(static_cast<const std::string &>(dev));
                proc.ignore(std::numeric_limits<std::streamsize>::max(), proc.widen('\n'));
            }

            return vec;
        }

        /// addr...

        template <int SIOC>
        std::string
        inet_addr() const
        {
            if (ioctl(_M_sock, SIOC, &_M_ifreq_io) != -1 ) {
                struct sockaddr_in *p = (struct sockaddr_in *)&_M_ifreq_io.ifr_addr;

                if(_M_ifreq_io.ifr_addr.sa_family == AF_INET) {
                    char dst[16];
                    return inet_ntop(AF_INET, reinterpret_cast<const void *>(&p->sin_addr), dst, sizeof(dst));
                }
            }
            return std::string();
        }

        std::string ipaddr() const
        { return inet_addr<SIOCGIFADDR>(); }
        
        std::string broadcast() const
        { return inet_addr<SIOCGIFBRDADDR>(); }

        std::string netmask() const
        { return inet_addr<SIOCGIFNETMASK>(); }

        std::string
        mac() const 
        {
            if (ioctl(_M_sock, SIOCGIFHWADDR, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFHWADDR");
            }
            struct ether_addr *eth_addr = (struct ether_addr *) & _M_ifreq_io.ifr_addr.sa_data;
            return ether_ntoa(eth_addr);
        }

        int 
        mtu() const
        {
            if (ioctl(_M_sock, SIOCGIFMTU, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMTU");
            }
            return _M_ifreq_io.ifr_mtu;
        }

        int 
        metric() const
        {
            if (ioctl(_M_sock, SIOCGIFMETRIC, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMETRIC");
            }
            return _M_ifreq_io.ifr_metric ? _M_ifreq_io.ifr_metric : 1;
        }

        int
        index() const
        {
            if (ioctl(_M_sock, SIOCGIFINDEX, &_M_ifreq_io) == -1 ) {
                throw std::runtime_error("ioctl: SIOCGIFMTU");
            }
            return _M_ifreq_io.ifr_ifindex;
        }

        /// ethtool...

        std::shared_ptr<ethtool_drvinfo>
        drvinfo() const
        {
            std::shared_ptr<ethtool_drvinfo> drvinfo = std::make_shared<ethtool_drvinfo>();
            uint32_t req = ETHTOOL_GDRVINFO;

            _M_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(drvinfo.get());
            strncpy(_M_ifreq_io.ifr_data, (char *) &req, sizeof(req));

            if (ioctl(_M_sock, SIOCETHTOOL, &_M_ifreq_io) == -1) {
                throw std::runtime_error("SIOCETHTOOL");
            } 
           
            return drvinfo;          
        }

        std::shared_ptr<ethtool_cmd> 
        command() const
        {
            std::shared_ptr<ethtool_cmd> ecmd = std::make_shared<ethtool_cmd>();

            ecmd->cmd = ETHTOOL_GSET;
            _M_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(ecmd.get());

            if (ioctl(_M_sock, SIOCETHTOOL, &_M_ifreq_io) == -1) {
                throw std::runtime_error("SIOCETHTOOL");
            } 
            return ecmd;   
        }

        bool
        link() const 
        {
            struct ethtool_value edata;
            edata.cmd = ETHTOOL_GLINK;

            _M_ifreq_io.ifr_data = reinterpret_cast<__caddr_t>(&edata);
            if (ioctl(_M_sock, SIOCETHTOOL, &_M_ifreq_io) == -1) {
                throw std::runtime_error("SIOCETHTOOL");
            }
            return edata.data;    
        }                 

        /// stats...

        std::pair<stat,stat>
        stats() const
        {
            std::ifstream proc("/proc/net/dev"); assert(proc);

            // skip the first couple of lines:
            proc.ignore(std::numeric_limits<std::streamsize>::max(), proc.widen('\n'));
            proc.ignore(std::numeric_limits<std::streamsize>::max(), proc.widen('\n'));
            
            devname_t dev;
            stat receive, transmit;
            while(proc >> dev)
            {
                if (static_cast<const std::string &>(dev) != _M_dev)
                {
                    proc.ignore(std::numeric_limits<std::streamsize>::max(), proc.widen('\n'));
                    continue;
                }
                int colon = proc.get(); assert(colon == ':');
                proc >> receive >> transmit;
                return std::make_pair(receive, transmit);
            }
            throw std::runtime_error(std::string(_M_dev).append(": unknown device"));
        }

        stat
        receive_stat() const
        {
            return stats().first;
        }

        stat 
        transmit_stat() const
        {
            return stats().second;
        }

        /// flags...

        struct flags_t
        {
            flags_t(short int v)
            : _M_value(v)
            {}

            template <typename Tp>
            static Tp bit(Tp i)
            { return (1<<(i-1)); }

            template <int N>
            bool is_set() const
            {
                return _M_value & bit(N);
            }

            std::string
            str() const
            {
                const char *if_flags[]= {
                    "UP", "BROADCAST", "DEBUG", "LOOPBACK", "PTP", "NOTRL", "RUNNING", "NOARP", 
                    "PROMIS", "ALLMULTI", "MASTER", "SLAVE", "MULTICAST", "PORTSEL", "AUTOMEDIA" };

                std::stringstream ret;
                for (int i=1;i<16;i++) 
                    if (_M_value & bit(i)) 
                        ret << if_flags[i-1] << ' ';
                return ret.str(); 
            }

            short int value() const
            { 
                return _M_value;
            }

        private:
            short int _M_value;
        };

        flags_t
        flags() const 
        {
            if (ioctl(_M_sock, SIOCGIFFLAGS, &_M_ifreq_io) < 0)
                throw std::runtime_error("SIOCGIFFLAGS");
            return flags_t(_M_ifreq_io.ifr_flags);
        }

        /// predicates...

        bool is_up() const
        { return flags().is_set<1>(); }

        bool is_broadcast() const
        { return flags().is_set<2>(); }

        bool is_debug() const
        { return flags().is_set<3>(); }
        
        bool is_loopback() const
        { return flags().is_set<4>(); }
                          
        bool is_ptp() const
        { return flags().is_set<5>(); }

        bool is_notrl() const
        { return flags().is_set<6>(); }

        bool is_running() const
        { return flags().is_set<7>(); }

        bool is_noarp() const
        { return flags().is_set<8>(); }

        bool is_promisc() const
        { return flags().is_set<9>(); }
        
        bool is_allmulti() const
        { return flags().is_set<10>(); }
        
        bool is_master() const
        { return flags().is_set<11>(); }
        
        bool is_slave() const
        { return flags().is_set<12>(); }
        
        bool is_multicast() const
        { return flags().is_set<13>(); }
        
        bool is_portsel() const
        { return flags().is_set<14>(); }  
        
        bool is_automedia() const
        { return flags().is_set<15>(); }
        
    private:
        mutable struct ifreq _M_ifreq_io;
        std::string _M_dev;
        int _M_sock;  // for ifreq IO
    };

} // namespace netdev
} // namespace more

#endif /* _NETDEV_HPP_ */
