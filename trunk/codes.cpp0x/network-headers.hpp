/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _NETWORK_HEADERS_HPP_
#define _NETWORK_HEADERS_HPP_ 

#include <netinet/ether.h>      // ethernet 
#include <netinet/ip.h>         // iphdr
#include <netinet/udp.h>        // updhdr
#include <netinet/tcp.h>        // tcphdr
#include <netinet/ip_icmp.h>    // icmphdr

#include <net/ethernet.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

#include <range_iterator.hpp>    // more!

#define MORE_NET_READER_RAW(_type, _member) \
_type \
_member() const \
{   \
    static_assert( std::is_convertible<decltype(m_header->_member), _type>::value, \
                   "member -> return non convertible type"); \
    return m_header->_member;  \
}

#define MORE_NET_READER(_type, _member) \
_type \
_member() const \
{   \
    static_assert( std::is_convertible<decltype(m_header->_member), _type>::value, \
                   "member -> return: non convertible"); \
    return ntoh(m_header->_member); \
}

#define MORE_NET_WRITER_RAW(_type, _member) \
void \
_member(const _type &value)\
{  \
    static_assert(std::is_convertible<_type, decltype(m_header->_member)>::value, \
                  "argument -> member: not convertible"); \
    m_header->_member = value; \
}

#define MORE_NET_WRITER(_type, _member) \
void \
_member(const _type value)\
{ \
    static_assert(std::is_convertible<_type, decltype(m_header->_member)>::value, \
                  "argument -> member: not convertible"); \
    m_header->_member = hton(value); \
}

namespace more { namespace net { 

    // extra basic headers...
    //

    struct vlan_header
    {
        uint16_t vlan_tag;
        uint16_t ether_type;	
    };

    namespace 
    {
        struct host_byte_order_t {};     // to read value in host order
        struct network_byte_order_t {};  // to read value in host order

        host_byte_order_t    host_byte_order = {};
        network_byte_order_t network_byte_order = {};
    }

    namespace details
    {
        //  slightly modified version cksum from TCP/IP Illustrated Vol. 2(1995) 
        //  by Gary R. Wright and W. Richard Stevens.  

        static inline
        uint32_t csum_partial(void *buf, int len, uint32_t sum) 
        {
            uint16_t *p = static_cast<uint16_t *>(buf);

            while(len > 1)
            {
                sum += *(p)++;
                if(sum & 0x80000000)   /* if high order bit set, fold */
                    sum = (sum & 0xFFFF) + (sum >> 16);
                len -= 2;
            }

            if(len) /* take care of left over byte */
                sum += *(unsigned char *)p;

            return sum;
        }

        static inline
        uint16_t csum_fold(uint32_t sum)
        {
            while(sum>>16)
                sum = (sum & 0xffff) + (sum >> 16);
            return ~sum;
        }

        template <typename Tp, size_t N>
        inline Tp hton(Tp value, std::integral_constant<size_t, N>)
        {
            return value;
        }
        template <typename Tp>
        inline Tp hton(Tp value, std::integral_constant<size_t, 4>)
        {
            return htonl(value);
        }
        template <typename Tp>
        inline Tp hton(Tp value, std::integral_constant<size_t, 2>)
        {
            return htons(value);
        }
        template <typename Tp, size_t N>
        inline Tp ntoh(Tp value, std::integral_constant<size_t, N>)
        {
            return value;
        }
        template <typename Tp>
        inline Tp ntoh(Tp value, std::integral_constant<size_t, 4>)
        {
            return ntohl(value);
        }
        template <typename Tp>
        inline Tp ntoh(Tp value, std::integral_constant<size_t, 2>)
        {
            return ntohs(value);
        }

    } // namespace details

    template <typename Tp>
    inline Tp hton(Tp n)
    {
        return details::hton(n, std::integral_constant<size_t, sizeof(Tp)>());
    }
    template <typename Tp>
    inline Tp ntoh(Tp n)
    {
        return details::ntoh(n, std::integral_constant<size_t, sizeof(Tp)>());
    }

    //////////////////////////////////////////////////////////
    // header traits
    //

    class ethernet;
    class eth802_1q;
    class ipv4;
    class tcp;
    class udp;
    class basic_icmp;

    template <typename Tp>
    struct header_traits ;

    template <>
    struct header_traits<ethernet>
    {
        static const int static_size = sizeof(ether_header);       // static size
        static const int min_size = sizeof(ether_header);          // min size
        static inline const char *name() { return "ether"; }
    };
    template <>
    struct header_traits<eth802_1q>
    {
        static const int static_size = sizeof(vlan_header);	        // static size
        static const int min_size = sizeof(vlan_header);  	        // min size
        static inline const char *name() { return "eth802_1q"; }
    };
    template <>
    struct header_traits<ipv4>
    {  
        static const int static_size = 0;                           // dynamic size
        static const int min_size = sizeof(iphdr);                  // min size
        static inline const char *name() { return "ipv4"; }
    };
    template <>
    struct header_traits<udp>
    {
        static const int static_size = sizeof(udphdr);              // static size
        static const int min_size = sizeof(udphdr);                 // min size
        static inline const char *name() { return "udp"; }
    };
    template <>
    struct header_traits<tcp>
    {
        static const int static_size = 0;                           // dynamic size
        static const int min_size = sizeof(tcphdr);                 // min size
        static inline const char *name() { return "tcp"; }
    };
    template <>
    struct header_traits<basic_icmp>
    {
        static const int static_size = sizeof(icmphdr);           // static size
        static const int min_size = sizeof(icmphdr);              // min size
        static inline const char *name() { return "basic_icmp"; }
    };

    //////////////////////////////////////////////////////////
    // header/const_header classes
    //////////////////////////////////////////////////////////

    template <typename Tp>
    class header
    {
        // static size headers...
        template <typename I, int N>
        void check(more::range_iterator_adapter<I> &it, std::integral_constant<int,N>)
        {
            if (it.capacity() < N)
                throw std::range_error(std::string(header_traits<typename std::remove_cv<Tp>::type>::name()).append("::static_size"));
            it += N; 
        }

        template <typename I, int N>
        void check(more::range_iterator_adapter<I> &it, size_t size, std::integral_constant<int,N>)
        {
            if ( it.capacity() < N)
                throw std::range_error(std::string(header_traits<typename std::remove_cv<Tp>::type>::name()).append("::static_size"));
            if ( size && size != N )
                throw std::range_error(std::string("size != ").
                                       append(header_traits<typename std::remove_cv<Tp>::type>::name()).append("::static_size"));
            it += N;
        }

        // dynamic size headers...
        template <typename I>
        void check(more::range_iterator_adapter<I> &it, std::integral_constant<int,0>)
        {
            if ( it.capacity() < header_traits<typename std::remove_cv<Tp>::type>::min_size )
                throw std::range_error(std::string(header_traits<typename std::remove_cv<Tp>::type>::name()).
                                       append("::size() [minimal size]"));
            it += m_header.size(it.capacity());
        }

        template <typename I>
        void check(more::range_iterator_adapter<I> &it, size_t size, std::integral_constant<int,0>)
        {
            ssize_t n = m_header.size(it.capacity(), size);
            if (it.capacity() < n)
                throw std::range_error(std::string(header_traits<typename std::remove_cv<Tp>::type>::name()).append("::size() [dynamic size]"));
            it += n;
        }

    public:
        template <typename It>
        header(more::range_iterator_adapter<It> &it)
        : m_header(&(*it))
        {
            static_assert(sizeof(typename more::range_iterator_adapter<It>::value_type) == 1, "invalid range_iterator"); 
            check(it, std::integral_constant<int, header_traits<typename std::remove_cv<Tp>::type>::static_size>());  
        }
        template <typename It>
        header(more::range_iterator_adapter<It> &it, size_t s /* set the header size */)
        : m_header(&(*it))
        {
            static_assert(sizeof(typename more::range_iterator_adapter<It>::value_type) == 1, "invalid range_iterator"); 
            check(it, s, std::integral_constant<int, header_traits<typename std::remove_cv<Tp>::type>::static_size>());  
        }

        Tp * 
        operator->()
        {
            return &m_header;
        }

        Tp &
        operator *()
        {
            return m_header;
        }

    private:
        Tp m_header;
    };

    template <typename Tp>
    class const_header
    {
        // static size headers...
        template <typename I, int N>
        void check(more::range_const_iterator_adapter<I> &it, std::integral_constant<int,N>)
        {
            if (it.capacity() < N)
                throw std::range_error(std::string(header_traits<typename std::remove_cv<Tp>::type>::name()).append("::static_size"));
            it += N; 
        }

        // dynamic size headers...
        template <typename I>
        void check(more::range_const_iterator_adapter<I> &it, std::integral_constant<int,0>)
        {
            if ( it.capacity() < header_traits<typename std::remove_cv<Tp>::type>::min_size )
                throw std::range_error(std::string(header_traits<typename std::remove_cv<Tp>::type>::name()).
                                       append("::size() [minimal size]"));
            it += m_header.size(it.capacity());
        }

    public:
        template <typename It>
        const_header(more::range_const_iterator_adapter<It> &it)
        : m_header(const_cast<
                   typename std::add_pointer<
                        typename std::remove_const<
                            typename std::remove_pointer<
                                typename more::range_const_iterator_adapter<It>::pointer>::type
                            >::type
                   >::type >(&*it))
        {
            static_assert(sizeof(typename more::range_const_iterator_adapter<It>::value_type) == 1, "invalid range_iterator"); 
            check(it, std::integral_constant<int, header_traits<typename std::remove_cv<Tp>::type>::static_size>());  
        }

        const Tp * 
        operator->()
        {
            return &m_header;
        }

        const Tp &
        operator *()
        {
            return m_header;
        }

    private:
        typename std::add_const<Tp>::type m_header;
    };

    //////////////////////////////////////////////////////////
    // protocol dissectors
    //////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////
    // ethernet 802.1

    class ethernet
    {
        ether_header *m_header;            

    private:
        friend class header<ethernet>;
        friend class const_header<ethernet>;

        size_t size(ssize_t) const
        {
            return size();
        }

        size_t size(ssize_t, size_t) = delete; 

    public:
        static const uint16_t type_8021q = 0x8100;
        
        ethernet(void *h)
        : m_header(static_cast<ether_header *>(h))
        {}

        size_t size() const
        {
            return sizeof(ether_header);
        }

        std::string
        dhost() const
        {
            char buf[24];
            ether_ntoa_r(reinterpret_cast<struct ether_addr *>(m_header->ether_dhost), buf);
            return std::string(buf); 
        }

        const uint8_t *
        dhost(network_byte_order_t) const
        {
            return m_header->ether_dhost;
        }

        void
        dhost(const char *a)
        {
            if (ether_aton_r(a, reinterpret_cast<struct ether_addr *>(m_header->ether_dhost)) == nullptr)
                throw std::runtime_error("dhost: ether_aton_r");
        }

        std::string
        shost() const
        {
            char buf[24];
            ether_ntoa_r(reinterpret_cast<struct ether_addr *>(m_header->ether_shost), buf);
            return std::string(buf); 
        }

        const uint8_t *
        shost(network_byte_order_t) const
        {
            return m_header->ether_shost;
        }

        void
        shost(const char *a)
        {
            if(ether_aton_r(a, reinterpret_cast<struct ether_addr *>(m_header->ether_shost)) == nullptr)
                throw std::runtime_error("shost: ether_aton_r");
        }

        uint16_t
        ether_type() const
        {
            return ntohs(m_header->ether_type);
        }

        void
        ether_type(uint16_t value)
        {
            m_header->ether_type = htons(value);
        }
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const ethernet& h)
    {
        return out << "[dhost=" << h.dhost() << 
        " shost=" << h.shost() << 
        " type="  << std::hex << h.ether_type() << std::dec <<  "]";
    }

    //////////////////////////////////////////////////////////
    // ethernet 802.1q

    class eth802_1q 
    {
        struct vlan_header
        {
            uint16_t vlan_tag;
            uint16_t ether_type;	
        };

        vlan_header * m_header;

    private:
        friend class header<eth802_1q>;
        friend class const_header<eth802_1q>;

        size_t size(ssize_t) const
        {
            return size();
        }
        
        size_t size(ssize_t, size_t) = delete; 

    public:

        eth802_1q(void *h)
        : m_header(static_cast<vlan_header *>(h))
        {} 

        size_t size() const
        {
            return sizeof(vlan_header);
        }

        uint16_t
        vlan_tag() const
        {
            return ntohs(m_header->vlan_tag);
        }

        void
        vlan_tag(uint16_t value)
        {
            m_header->vlan_tag = htons(value);
        }

        uint16_t
        ether_type() const
        {
            return ntohs(m_header->ether_type);
        }

        void
        ether_type(uint16_t value)
        {
            m_header->ether_type = htons(value);
        }

    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const eth802_1q & h)
    {
        return out << "[vlan_tag=" << std::hex << h.vlan_tag() << 
        " ether_type="  << h.ether_type() << std::dec <<  "]";
    }

    //////////////////////////////////////////////////////////
    // ipv4 header

    class ipv4
    {
        iphdr * m_header;

    private:
        friend class header<ipv4>;
        friend class const_header<ipv4>;

        size_t size(ssize_t cap) const
        {
            if ( cap != -1 && cap < 1 )
                throw std::range_error("ip::size()");
            return size();
        }

        size_t size(ssize_t cap, size_t size)
        {
            if ( cap < 1 )
                throw std::range_error("ip::size()");
            if (size < 20 || size > 60 || size % 4)
                throw std::range_error("ip::size(): bad lenght");
            this->ihl(size>>2);
            return this->ihl()<<2;
        } 

    public:
        ipv4(void *h)
        : m_header(static_cast<iphdr *>(h))
        {} 

        size_t size() const
        {
            return this->ihl()<<2;
        }

        MORE_NET_READER_RAW(int, version);
        MORE_NET_WRITER_RAW(int, version);

        MORE_NET_READER_RAW(int, ihl);       // header len in words: 5 -> * 4 = 20 bytes
    private:
        MORE_NET_WRITER_RAW(int, ihl);       // header len in words: 5 -> * 4 = 20 bytes
    public:
        MORE_NET_READER(uint8_t, tos);
        MORE_NET_WRITER(uint8_t, tos);

        MORE_NET_READER(uint16_t, tot_len);
        MORE_NET_WRITER(uint16_t, tot_len);

        MORE_NET_READER(uint16_t, id);
        MORE_NET_WRITER(uint16_t, id);

        MORE_NET_READER(uint16_t, frag_off);
        MORE_NET_WRITER(uint16_t, frag_off);

        MORE_NET_READER(uint8_t, ttl);
        MORE_NET_WRITER(uint8_t, ttl);

        MORE_NET_READER(uint8_t, protocol);
        MORE_NET_WRITER(uint8_t, protocol);

        MORE_NET_READER(uint16_t, check);
        MORE_NET_WRITER(uint16_t, check);

        void
        chksum_update()
        {
            m_header->check = 0;
            m_header->check = details::csum_fold(details::csum_partial((uint16_t *)m_header, this->size(),0)); 
        }

        bool
        chksum_verify() const
        {
            return details::csum_fold(details::csum_partial((uint16_t *)m_header, this->size(),0)) == 0;
        }

        std::string
        saddr() const
        {
            char buf[16];
            inet_ntop(AF_INET, &m_header->saddr, buf, sizeof(buf));
            return std::string(buf);
        }

        uint32_t
        saddr(network_byte_order_t) const
        {
            return m_header->saddr;
        }
        uint32_t
        saddr(host_byte_order_t) const
        {
            return ntoh(m_header->saddr);
        }

        void
        saddr(const char *ip_addr)
        {
            if (inet_pton(AF_INET,ip_addr, &m_header->saddr) <= 0)
                throw std::runtime_error("ipv4::saddr: inet_pton");
        }

        void
        saddr(uint32_t addr)
        {
            m_header->saddr = htonl(addr);
        }

        std::string
        daddr() const
        {            
            char buf[16];
            inet_ntop(AF_INET, &m_header->daddr, buf, sizeof(buf));
            return std::string(buf);
        }

        uint32_t
        daddr(network_byte_order_t) const
        {
            return m_header->daddr;
        }
        uint32_t
        daddr(host_byte_order_t) const
        {
            return ntoh(m_header->daddr);
        }

        void
        daddr(const char *ip_addr)
        {
            if (inet_pton(AF_INET,ip_addr, &m_header->daddr) <= 0)
                throw std::runtime_error("ipv4::saddr: inet_pton");
        }

        void
        daddr(uint32_t addr)
        {
            m_header->daddr = htonl(addr);
        }
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const ipv4 & h)
    {
        return out << std::hex << 
        "[ihl="      << h.ihl() << 
        " ver:"      << h.version() <<
        " tos="      << static_cast<uint16_t>(h.tos()) << std::dec << 
        " tot_len="  << h.tot_len() << std::hex << 
        " id="       << h.id() << std::dec << 
        " frag_off=" << h.frag_off() <<
        " ttl="      << static_cast<uint16_t>(h.ttl()) << std::hex << 
        " proto="    << static_cast<uint16_t>(h.protocol()) << 
        " csum="     << h.check() <<
        " saddr="    << h.saddr() << 
        " daddr="    << h.daddr() << "]" << std::dec;
    }

    //////////////////////////////////////////////////////////
    // tcp header

    class tcp
    {
        tcphdr * m_header;

    private:
        friend class header<tcp>;
        friend class const_header<tcp>;

        size_t size(ssize_t cap) const
        {
            // throw if there are no sufficient bytes to calculate the size
            if ( cap != -1 && cap < 13 )
                throw std::range_error("tcp::size()");

            return size();
        }
        
        size_t size(ssize_t cap, size_t size)
        {
            // throw if there are no sufficient bytes to set this size
            if ( cap < 13 )
                throw std::range_error("tcp::size()");

            if (size < 20 || size > 60 || size % 4)
                throw std::range_error("tcp::size(): bad lenght");
            
            this->doff(size>>2);
            return this->doff()<<2;
        } 

    public:
        struct pseudo_header
        {
            uint32_t saddr;
            uint32_t daddr;
            uint8_t  zero;
            uint8_t  protocol;
            uint16_t length;
        } __attribute__((packed));

        tcp(void *h)
        : m_header(static_cast<tcphdr *>(h))
        {} 

        size_t size() const
        {
            return this->doff()<<2;
        }

        MORE_NET_READER(uint16_t, source);
        MORE_NET_WRITER(uint16_t, source);

        MORE_NET_READER(uint16_t, dest);
        MORE_NET_WRITER(uint16_t, dest);

        MORE_NET_READER(uint32_t, seq);
        MORE_NET_WRITER(uint32_t, seq);

        MORE_NET_READER(uint32_t, ack_seq);
        MORE_NET_WRITER(uint32_t, ack_seq);


        MORE_NET_READER(uint16_t,doff);
    private:
        MORE_NET_WRITER(uint16_t,doff);  // the size, in case, is set during construction
    public:

        MORE_NET_READER(uint16_t, res1);
        MORE_NET_WRITER(uint16_t, res1);

        MORE_NET_READER(uint16_t, res2);
        MORE_NET_WRITER(uint16_t, res2);

        bool 
        cwr() const
        {
            return m_header->res2 & 1;
        }

        void
        cwr(bool value)
        {
            m_header->res2 = ( m_header->res2 & 2 ) | value;
        } 

        bool 
        ece() const
        {
            return m_header->res2 & 2;
        }

        void
        ece(bool value)
        {
            m_header->res2 = ( m_header->res2 & 1 ) | (value<<1);
        } 

        MORE_NET_READER_RAW(bool, urg);
        MORE_NET_WRITER_RAW(bool, urg);

        MORE_NET_READER_RAW(bool, ack);
        MORE_NET_WRITER_RAW(bool, ack);

        MORE_NET_READER_RAW(bool, psh);
        MORE_NET_WRITER_RAW(bool, psh);

        MORE_NET_READER_RAW(bool, rst);
        MORE_NET_WRITER_RAW(bool, rst);

        MORE_NET_READER_RAW(bool, syn);
        MORE_NET_WRITER_RAW(bool, syn);

        MORE_NET_READER_RAW(bool, fin);
        MORE_NET_WRITER_RAW(bool, fin);

        void
        flags_reset() 
        {
            *(reinterpret_cast<char *>(m_header) + 13) = '\0';
        }

        uint8_t 
        flags() const 
        {
            return *(reinterpret_cast<char *>(m_header) + 13);
        }

        MORE_NET_READER(uint16_t, window);
        MORE_NET_WRITER(uint16_t, window);

        MORE_NET_READER(uint16_t, check);
        MORE_NET_WRITER(uint16_t, check);

        void 
        chksum_update(const ipv4 & ip, ssize_t data_len) 
        {
            ssize_t tcp_data_len = ip.tot_len() - ip.size() - this->size();
            if (tcp_data_len < 0)
                throw std::runtime_error("tcp::checksum: tcp_data_len");

            if (data_len < tcp_data_len)
                throw std::runtime_error("tcp::checksum: missing bytes");

            chksum_update(ip.saddr(host_byte_order), ip.daddr(host_byte_order), tcp_data_len); 
        }

        bool 
        chksum_verify(const ipv4 & ip, ssize_t data_len) const
        {
            ssize_t tcp_data_len = ip.tot_len() - ip.size() - this->size();
            if (tcp_data_len < 0)
                throw std::runtime_error("tcp::checksum: tcp_data_len");

            if (data_len < tcp_data_len)
                std::clog << "tcp::checksum: missing bytes, checksum unverifiable" << std::endl;

            return chksum_verify(ip.saddr(host_byte_order), ip.daddr(host_byte_order), std::min(data_len,tcp_data_len)); 
        }

    private:
        void
        chksum_update(uint32_t src, uint32_t dst, int tcp_data_len)
        {
            pseudo_header ph;

            ph.saddr = htonl(src);
            ph.daddr = htonl(dst);
            ph.zero  = 0;
            ph.protocol = IPPROTO_TCP;
            ph.length = htons(this->size() + tcp_data_len); // tcp header + tcp_data 

            uint32_t sum = details::csum_partial((uint16_t *)& ph, sizeof(pseudo_header), 0); 

            m_header->check = 0;
            m_header->check = details::csum_fold(details::csum_partial((uint16_t *)m_header, this->size() + tcp_data_len, sum) ); 
        }


        bool
        chksum_verify(uint32_t src, uint32_t dst, int tcp_data_len) const
        {
            pseudo_header ph;

            ph.saddr = htonl(src);
            ph.daddr = htonl(dst);
            ph.zero  = 0;
            ph.protocol = IPPROTO_TCP;
            ph.length = htons(this->size() + tcp_data_len); // tcp header + tcp_data 

            uint32_t sum = details::csum_partial((uint16_t *)& ph, sizeof(pseudo_header), 0); 
            return details::csum_fold(details::csum_partial((uint16_t *)m_header, this->size() + tcp_data_len, sum) ) == 0; 
        }

    public:
        MORE_NET_READER(uint16_t, urg_ptr);
        MORE_NET_WRITER(uint16_t, urg_ptr);

    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const tcp & h)
    {
        return out << "[source=" << h.source() << 
        " dest=" << h.dest() << 
        " seq=" << h.seq() << 
        " ack_seq=" << h.ack_seq() << 
        " doff=" << h.doff() <<
        " flags=" << ( h.cwr() ? "W" : "" ) <<
        ( h.ece() ? "E" : "" ) <<
        ( h.urg() ? "U" : "" ) <<
        ( h.ack() ? "A" : "" ) <<
        ( h.psh() ? "P" : "" ) <<
        ( h.rst() ? "R" : "" ) <<
        ( h.syn() ? "S" : "" ) <<
        ( h.fin() ? "F" : "" ) <<

        " window="  << h.window() << 
        " csum="    << std::hex << h.check() << std::dec << 
        " urg=" << h.urg_ptr() << "]";
    }

    //////////////////////////////////////////////////////////
    // udp header

    class udp
    {
        udphdr * m_header;

    private:
        friend class header<udp>;
        friend class const_header<udp>;

        size_t size(ssize_t) const
        {
            return size();
        }

        size_t size(ssize_t, size_t) = delete; 

    public:
        udp(void *h)
        : m_header(static_cast<udphdr *>(h))
        {} 

        size_t size() const
        {
            return sizeof(udphdr);
        }

        MORE_NET_READER(uint16_t,source);
        MORE_NET_WRITER(uint16_t,source);

        MORE_NET_READER(uint16_t,dest);
        MORE_NET_WRITER(uint16_t,dest);

        MORE_NET_READER(uint16_t,len);
        MORE_NET_WRITER(uint16_t,len);

        MORE_NET_READER(uint16_t,check);
        MORE_NET_WRITER(uint16_t,check);
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const udp & h)
    {
        return out << "[source=" << h.source() << 
        " dest="  << h.dest() << 
        " len="   << h.len() <<  
        " csum="  << std::hex << h.check() << std::dec << "]";
    }

    //////////////////////////////////////////////////////////
    // basic icmp header

    class basic_icmp 
    {
        icmphdr * m_header;

    private:
        friend class header<icmp>;
        friend class const_header<icmp>;

        size_t size(ssize_t) const
        {
            return size();
        }
        
        size_t size(ssize_t, size_t) = delete; 

    public:
        basic_icmp(void *h)
        : m_header(static_cast<icmphdr *>(h))
        {} 

        size_t size() const
        {
            return sizeof(icmphdr);
        }

        //////////////////////////////////////////////////////

        MORE_NET_READER(uint8_t,type);   // uint8_t type();
        MORE_NET_WRITER(uint8_t,type);   // void type(uint8_t value);

        MORE_NET_READER(uint8_t,code);
        MORE_NET_WRITER(uint8_t,code);

        MORE_NET_READER(uint16_t,checksum);
        MORE_NET_WRITER(uint16_t,checksum);

    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const basic_icmp & h)
    {
        return out << std::hex << 
        "[type=" << static_cast<int>(h.type()) << 
        " code=" << static_cast<int>(h.code()) << 
        " csum=" << h.checksum() << std::dec << "]";
    }

} // namespace net
} // namespace more

#undef MORE_NET_READER_RAW 
#undef MORE_NET_READER
#undef MORE_NET_WRITER_RAW
#undef MORE_NET_WRITER

#endif /* _NETWORK_HEADERS_HPP_ */
