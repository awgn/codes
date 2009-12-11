/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _NET_HEADERS_HH_
#define _NET_HEADERS_HH_ 

#include <tr1/type_traits>
#include <iostream>
#include <string>

#include <netinet/ether.h>
#include <net/ethernet.h>
#include <arpa/inet.h>

namespace more {

    //////////////////////////////////////////////////////////
    // remove_const_if metafunction 

    template <bool V, typename T>
    struct remove_const_if
    {
        typedef typename std::tr1::remove_const<T>::type type;
    };

    template <typename T>
    struct remove_const_if<false, T>
    {
        typedef T type;
    };

    //////////////////////////////////////////////////////////
    // generic network header

    template <typename T>
    class header
    {
    public:
        template <typename P>
        header(P *p)
        : _M_value( const_cast< 
                        typename more::remove_const_if< 
                            std::tr1::is_const<T>::value, P
                            >::type * 
                        >(p))
        {}

        T * 
        operator->()
        {
            return &_M_value;
        }

        T &
        operator *()
        {
            return _M_value;
        }

    private:
        T _M_value;
    };

} // namespace more

namespace net {

    //////////////////////////////////////////////////////////
    // ethernet header

    class ethernet
    {
    public:

        template <typename T>
        ethernet(T *h)
        : _M_h(reinterpret_cast<ether_header *>(h))
        {} 

        std::string
        dhost() const
        {
            char buf[24];
            ether_ntoa_r(reinterpret_cast<struct ether_addr *>(_M_h->ether_dhost),buf);
            return std::string(buf); 
        }

        void
        dhost(const std::string &a)
        {
            ether_aton_r(a.c_str(), reinterpret_cast<struct ether_addr *>(_M_h->ether_dhost) );
        }

        std::string
        shost() const
        {
            char buf[24];
            ether_ntoa_r(reinterpret_cast<struct ether_addr *>(_M_h->ether_shost),buf);
            return std::string(buf); 
        }

        void
        shost(const std::string &a)
        {
            ether_aton_r(a.c_str(), reinterpret_cast<struct ether_addr *>(_M_h->ether_shost) );
        }

        u_int16_t
        ether_type() const
        {
            return ntohs(_M_h->ether_type);
        }

        void
        ether_type(u_int16_t value)
        {
            _M_h->ether_type = htons(value);
        }

        static ssize_t
        size()
        {
            return sizeof(ether_header);
        }

        ssize_t
        len() const
        {
            return sizeof(ether_header);
        }

    private:
        ether_header * _M_h;
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const ethernet & h)
    {
        return out << "ether[ dhost=" << h.dhost() << 
                            " shost=" << h.shost() << 
                            " type=0x" << std::hex << h.ether_type() << std::dec <<  " ]";
    }

    class ipv4
    {};

    class ipv6
    {};

    class icmp
    {};

    class udp
    {};

    class tcp
    {};

} // namespace net 

#endif /* _NET_HEADERS_HH_ */
