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

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>

#include <net/ethernet.h>
#include <arpa/inet.h>

#include <cursor.hh>        // more!
#include <static_assert.hh> // more!

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <tr1/type_traits>
namespace std { using namespace std::tr1; }
#else
#include <type_traits>
#endif

#include <iostream>
#include <string>
#include <stdexcept>
#include <algorithm>

namespace more {

    namespace header_helper {

        //////////////////////////////////////////////////////////
        // useful metafunctions...

        template <bool V, typename T>
        struct remove_const_if
        {
            typedef typename std::remove_const<T>::type type;
        };

        template <typename T>
        struct remove_const_if<false, T>
        {
            typedef T type;
        };

        template <bool V, typename T>
        struct add_const_if
        {
            typedef typename std::add_const<T>::type type;
        };

        template <typename T>
        struct add_const_if<false, T>
        {
            typedef T type;
        };

        template <int N>
        struct int2type
        {
            enum { value = N };
        };

    }

    //////////////////////////////////////////////////////////
    // generic network header

    template <typename T>
    class header
    {
        ////////////////////////////////////////////////
        // ctor for static size headers.. ie: ethernet

        template <typename P, int N>
        void ctor(more::cursor<P> &cur, header_helper::int2type<N>)
        {
            if (cur.size() < N)
                throw std::range_error(T::__name::str().append("::static_size"));
           
            cur += N; 
        }

        template <typename P, int N>
        void ctor(more::cursor<P> &cur, ssize_t size, header_helper::int2type<N>)
        {
            if ( cur.size() < N)
                throw std::range_error(T::__name::str().append("::static_size"));
            
            if ( size && size != N )
                throw std::range_error(std::string("size != ").append(T::__name::str()).append("::static_size"));

            cur += N;
        }

        ///////////////////////////////////////////
        // ctor for dynamic size headers.. ie: ip 

        template <typename P>
        void ctor(more::cursor<P> &cur, header_helper::int2type<0>)
        {
            if ( cur.size() < T::__min_size )
                throw std::range_error(std::string(T::__name::str()).append("::size() [minimal size]"));

            ssize_t n = _M_value.size(cur.size());
            cur += n;
        }

        template <typename P>
        void ctor(more::cursor<P> &cur, ssize_t size, header_helper::int2type<0>)
        {
            ssize_t n = _M_value.size(cur.size(), size);
            if (cur.size() < n)
                throw std::range_error(std::string(T::__name::str()).append("::size() [dynamic size]"));
            cur += n;
        }

    public:
        template <typename P>
        header(more::cursor<P> &cur)
        : _M_value( const_cast< 
                        typename header_helper::remove_const_if< 
                            std::is_const<T>::value, P
                            >::type * >(cur.cur()))
        {
            static_assert(sizeof(P) == 1, multi_bytes_cursor_not_allowed);
            ctor(cur, header_helper::int2type<T::__static_size>());
        }

        template <typename P>
        header(more::cursor<P> & cur, ssize_t size /* set the header size */ )
        : _M_value( const_cast< 
                        typename header_helper::remove_const_if< 
                            std::is_const<T>::value, P
                            >::type * >(cur.cur()))
        {
            static_assert(sizeof(P) == 1, multi_bytes_cursor_not_allowed);
            ctor(cur, size, header_helper::int2type<T::__static_size>());
        }


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

#define attr_reader(_type, _member) \
    _type \
    _member() const\
    { return _H_->_member; }

#define attr_reader_uint16(_member) \
    uint16_t \
    _member() const\
    { return ntohs(_H_->_member); }

#define attr_reader_uint32(_member) \
    uint32_t \
    _member() const\
    { return ntohl(_H_->_member); }

#define attr_writer(_type, _member) \
    void \
    _member(const _type &value)\
    { _H_->_member = value; }

#define attr_writer_uint16(_member) \
    void \
    _member(const uint16_t value)\
    { _H_->_member = htons(value); }

#define attr_writer_uint32(_member) \
    void \
    _member(const uint32_t value)\
    { _H_->_member = htonl(value); }


namespace net {

    struct raw {};  // to read raw fields

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
            sum = (sum & 0xFFFF) + (sum >> 16);
        return ~sum;
    }

    //////////////////////////////////////////////////////////
    // ethernet header
    //////////////////////////////////////////////////////////

    class ethernet
    {
    public:
        static const int   __static_size = sizeof(ether_header);    // static size
        static const int   __min_size = sizeof(ether_header);       // min size

        struct __name
        {
            static inline
            std::string str() 
            {
                return "ether";
            }
        };

        friend class more::header<ethernet>;

        template <typename T>
        ethernet(T *h)
        : _H_(reinterpret_cast<ether_header *>(h))
        {} 

        // for static-sized headers, just a size() method suffices...

        ssize_t
        size(ssize_t bytes = -1, ssize_t s = 0) const
        {
            // if ( bytes != -1 && bytes < sizeof(ether_header) )
            //    throw std::range_error("ethernet::size()");
            return sizeof(ether_header);
        }

        //////////////////////////////////////////////////////

        std::string
        dhost() const
        {
            char buf[24];
            ether_ntoa_r(reinterpret_cast<struct ether_addr *>(_H_->ether_dhost),buf);
            return std::string(buf); 
        }
    
        const uint8_t *
        dhost(raw) const
        {
            return _H_->ether_dhost;
        }

        void
        dhost(const std::string &a)
        {
            ether_aton_r(a.c_str(), reinterpret_cast<struct ether_addr *>(_H_->ether_dhost));
        }

        std::string
        shost() const
        {
            char buf[24];
            ether_ntoa_r(reinterpret_cast<struct ether_addr *>(_H_->ether_shost),buf);
            return std::string(buf); 
        }

        const uint8_t *
        shost(raw) const
        {
            return _H_->ether_shost;
        }

        void
        shost(const std::string &a)
        {
            ether_aton_r(a.c_str(), reinterpret_cast<struct ether_addr *>(_H_->ether_shost));
        }

        uint16_t
        ether_type() const
        {
            return ntohs(_H_->ether_type);
        }

        void
        ether_type(uint16_t value)
        {
            _H_->ether_type = htons(value);
        }

    private:
        ether_header * _H_;
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const ethernet & h)
    {
        return out << "[dhost=" << h.dhost() << 
                       " shost=" << h.shost() << 
                       " type="  << std::hex << h.ether_type() << std::dec <<  "]";
    }

    //////////////////////////////////////////////////////////
    // ipv4 header
    //////////////////////////////////////////////////////////

    class ipv4
    {
    public:
        static const int __static_size = 0;            // dynamic size
        static const int __min_size = sizeof(iphdr);   // min size

        struct __name
        {
            static inline
            std::string str() 
            {
                return "ipv4";
            }
        };

        friend class ::more::header<ipv4>;

        template <typename T>
        ipv4(T *h)
        : _H_(reinterpret_cast<iphdr *>(h))
        {} 

        ssize_t
        size(ssize_t bytes = -1) const
        {
            // throw if there are no sufficient bytes to calc the size

            if ( bytes != -1 && bytes < 1 )
                throw std::range_error("ip::size()");

            return this->ihl()<<2;
        }

    private:
        ssize_t
        size(ssize_t bytes, ssize_t ns /* new size */) 
        {
            // throw if there are no sufficient bytes to calc the size

            if ( bytes < 1 )
                throw std::range_error("ip::size()");

            if (ns < 20 || ns > 60 || ns % 4)
                throw std::range_error("ip::size(): bad lenght");
            
            this->ihl(ns>>2);
            return this->ihl()<<2;
        }

    public:
        attr_reader(int,version);
        attr_writer(int,version);
    
        attr_reader(int,ihl);       // header len in words: 5 -> * 4 = 20 bytes
    private:
        attr_writer(int,ihl);       // the size, in case, is set during construction  
    public:

        attr_reader(uint8_t,tos);
        attr_writer(uint8_t,tos);

        attr_reader_uint16(tot_len);
        attr_writer_uint16(tot_len);

        attr_reader_uint16(id);
        attr_writer_uint16(id);

        attr_reader_uint16(frag_off);
        attr_writer_uint16(frag_off);

        attr_reader(uint8_t,ttl);
        attr_writer(uint8_t,ttl);

        attr_reader(uint8_t,protocol);
        attr_writer(uint8_t,protocol);

        attr_reader_uint16(check);
        attr_writer_uint16(check);

        void
        check_update()
        {
            _H_->check = 0;
            _H_->check = csum_fold(csum_partial((uint16_t *)_H_, this->size(),0)); 
        }

        bool
        check_verify() const
        {
            return csum_fold(csum_partial((uint16_t *)_H_, this->size(),0)) == 0;
        }

        std::string
        saddr() const
        {
            char buf[16];
            inet_ntop(AF_INET, &_H_->saddr, buf, sizeof(buf));
            return std::string(buf);
        }
        
        uint32_t
        saddr(raw) const
        {
            return _H_->saddr;
        }

        void
        saddr(const std::string &ip_addr)
        {
            if (inet_pton(AF_INET,ip_addr.c_str(), &_H_->saddr) <= 0)
               throw std::runtime_error("ipv4::saddr: inet_pton");
        }

        std::string
        daddr() const
        {            
            char buf[16];
            inet_ntop(AF_INET, &_H_->daddr, buf, sizeof(buf));
            return std::string(buf);
        }
 
        uint32_t
        daddr(raw) const
        {
            return _H_->daddr;
        }
       
        void
        daddr(const std::string &ip_addr)
        {
            if (inet_pton(AF_INET,ip_addr.c_str(), &_H_->daddr) <= 0)
               throw std::runtime_error("ipv4::saddr: inet_pton");
        }

    private:
        iphdr * _H_;
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
    // udp header
    //////////////////////////////////////////////////////////

    class udp
    {
    public:
        static const int __static_size = sizeof(udphdr);   // static size
        static const int __min_size = sizeof(udphdr);      // min size

        struct __name
        {
            static inline
            std::string str() 
            {
                return "udp";
            }
        };

        friend class more::header<udp>;

        template <typename T>
        udp(T *h)
        : _H_(reinterpret_cast<udphdr *>(h))
        {} 

        ssize_t
        size(ssize_t bytes = -1, ssize_t s = 0) const
        {
            return sizeof(udphdr);
        }

        //////////////////////////////////////////////////////

        attr_reader_uint16(source);
        attr_writer_uint16(source);

        attr_reader_uint16(dest);
        attr_writer_uint16(dest);

        attr_reader_uint16(len);
        attr_writer_uint16(len);

        attr_reader_uint16(check);
        attr_writer_uint16(check);

    private:
        udphdr * _H_;
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
    // tcp header
    //////////////////////////////////////////////////////////

    class tcp
    {
    public:

        struct pseudo_header
        {
            uint32_t saddr;
            uint32_t daddr;
            uint8_t  zero;
            uint8_t  protocol;
            uint16_t length;
        } __attribute__((packed));

        static const int __static_size = 0;                // dynamic size
        static const int __min_size = sizeof(tcphdr);      // min size

        struct __name
        {
            static inline
            std::string str() 
            {
                return "tcp";
            }
        };

        friend class more::header<tcp>;

        template <typename T>
        tcp(T *h)
        : _H_(reinterpret_cast<tcphdr *>(h))
        {} 

        ssize_t
        size(ssize_t bytes = -1) const
        {
            // throw if there are no sufficient bytes to calc the size

            if ( bytes != -1 && bytes < 13 )
                throw std::range_error("tcp::size()");

            return this->doff()<<2;
        }

    private:
        ssize_t
        size(ssize_t bytes, ssize_t ns /* new size */) 
        {
            // throw if there are no sufficient bytes to calc the size

            if ( bytes < 13 )
                throw std::range_error("tcp::size()");

            if (ns < 20 || ns > 60 || ns % 4)
                throw std::range_error("tcp::size(): bad lenght");
            
            this->doff(ns>>2);
            return this->doff()<<2;
        }

    public:
        attr_reader_uint16(source);
        attr_writer_uint16(source);

        attr_reader_uint16(dest);
        attr_writer_uint16(dest);

        attr_reader_uint32(seq);
        attr_writer_uint32(seq);

        attr_reader_uint32(ack_seq);
        attr_writer_uint32(ack_seq);


        attr_reader(uint16_t,doff);
    private:
        attr_writer(uint16_t,doff);  // the size, in case, is set during construction
    public:

        attr_reader(uint16_t,res1);
        attr_writer(uint16_t,res1);

        attr_reader(uint16_t,res2);
        attr_writer(uint16_t,res2);

        bool 
        cwr() const
        {
            return _H_->res2 & 1;
        }

        void
        cwr(bool value)
        {
            _H_->res2 = ( _H_->res2 & 2 ) | value;
        } 

        bool 
        ece() const
        {
            return _H_->res2 & 2;
        }

        void
        ece(bool value)
        {
            _H_->res2 = ( _H_->res2 & 1 ) | (value<<1);
        } 

        attr_reader(bool,urg);
        attr_writer(bool,urg);

        attr_reader(bool,ack);
        attr_writer(bool,ack);

        attr_reader(bool,psh);
        attr_writer(bool,psh);

        attr_reader(bool,rst);
        attr_writer(bool,rst);

        attr_reader(bool,syn);
        attr_writer(bool,syn);

        attr_reader(bool,fin);
        attr_writer(bool,fin);

        void
        flags_reset() 
        {
            *(reinterpret_cast<char *>(_H_) + 13) = '\0';

            // cwr(false);
            // ece(false);
            // urg(false);
            // ack(false);
            // psh(false);
            // rst(false);
            // syn(false);
            // fin(false);
        }

        uint8_t 
        flags(raw) const 
        {
            return *(reinterpret_cast<char *>(_H_) + 13);
        }


        attr_reader_uint16(window);
        attr_writer_uint16(window);

        attr_reader_uint16(check);
        attr_writer_uint16(check);

        void 
        check_update(const ipv4 & ip, ssize_t data_len) 
        {
            ssize_t tcp_data_len = ip.tot_len() - ip.size() - this->size();
            if (tcp_data_len < 0)
                throw std::runtime_error("tcp::checksum: tcp_data_len");

            if (data_len < tcp_data_len)
                throw std::runtime_error("tcp::checksum: missing bytes");

            check_update(ip.saddr(raw()), ip.daddr(raw()), tcp_data_len); 
        }

        bool 
        check_verify(const ipv4 & ip, ssize_t data_len) const
        {
            ssize_t tcp_data_len = ip.tot_len() - ip.size() - this->size();
            if (tcp_data_len < 0)
                throw std::runtime_error("tcp::checksum: tcp_data_len");

            if (data_len < tcp_data_len)
                std::clog << "tcp::checksum: missing bytes, checksum unverifiable" << std::endl;

            return check_verify(ip.saddr(raw()), ip.daddr(raw()), std::min(data_len,tcp_data_len)); 
        }

    private:
        void
        check_update(uint32_t src, uint32_t dst, int tcp_data_len)
        {
            pseudo_header ph;
            
            ph.saddr = src;
            ph.daddr = dst;
            ph.zero  = 0;
            ph.protocol = IPPROTO_TCP;
            ph.length = htons(this->size() + tcp_data_len); // tcp header + tcp_data 
           
            uint32_t sum = csum_partial((uint16_t *)& ph, sizeof(pseudo_header), 0); 

            _H_->check = 0;
            _H_->check = csum_fold(csum_partial((uint16_t *)_H_, this->size() + tcp_data_len, sum) ); 
        }


        bool
        check_verify(uint32_t src, uint32_t dst, int tcp_data_len) const
        {
            pseudo_header ph;
            
            ph.saddr = src;
            ph.daddr = dst;
            ph.zero  = 0;
            ph.protocol = IPPROTO_TCP;
            ph.length = htons(this->size() + tcp_data_len); // tcp header + tcp_data 
           
            uint32_t sum = csum_partial((uint16_t *)& ph, sizeof(pseudo_header), 0); 
            return csum_fold(csum_partial((uint16_t *)_H_, this->size() + tcp_data_len, sum) ) == 0; 
        }

    public:
        attr_reader_uint16(urg_ptr);
        attr_writer_uint16(urg_ptr);

    private:
        tcphdr * _H_;
    
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
    // basic icmp header
    //////////////////////////////////////////////////////////

    class icmp 
    {
    public:
        static const int __static_size = sizeof(icmphdr);   // static size
        static const int __min_size = sizeof(icmphdr);      // min size

        struct __name
        {
            static inline
            std::string str() 
            {
                return "icmp";
            }
        };


        friend class more::header<icmp>;

        template <typename T>
        icmp(T *h)
        : _H_(reinterpret_cast<icmphdr *>(h))
        {} 

        ssize_t
        size(ssize_t bytes = -1, ssize_t s = 0) const
        {
            return sizeof(icmphdr);
        }

        //////////////////////////////////////////////////////

        attr_reader(uint8_t,type);
        attr_writer(uint8_t,type);

        attr_reader(uint8_t,code);
        attr_writer(uint8_t,code);

        attr_reader_uint16(checksum);
        attr_writer_uint16(checksum);

    private:
        icmphdr * _H_;
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const icmp & h)
    {
        return out << std::hex << 
                       "[type=" << static_cast<int>(h.type()) << 
                       " code=" << static_cast<int>(h.code()) << 
                       " csum=" << h.checksum() << std::dec << "]";
    }

    //////////////////////////////////////////////////////////
    // NP batch frame header
    //////////////////////////////////////////////////////////

    static const uint16_t NP_PKT_TYPE = 0x9000;

    class np_packet 
    {
        struct np_packet_hdr 
        {
            uint16_t reserved;
            uint16_t flow_id;
            uint16_t frag_len;
            uint16_t pack_len;

            uint32_t tstamp_lo;
            uint32_t tstamp_hi;

        } __attribute__((packed));

    public:
        static const int __static_size = sizeof(np_packet_hdr);   // static size
        static const int __min_size = sizeof(np_packet_hdr);      // min size

        struct __name
        {
            static inline
            std::string str() 
            {
                return "np_mon";
            }
        };

        friend class more::header<np_packet>;

        template <typename T>
        np_packet(T *h)
        : _H_(reinterpret_cast<np_packet_hdr *>(h))
        {} 

        ssize_t
        size(ssize_t bytes = -1, ssize_t s = 0) const
        {
            return sizeof(np_packet_hdr);
        }

        //////////////////////////////////////////////////////

        attr_reader_uint16(reserved);
        attr_writer_uint16(reserved);

        attr_reader_uint16(flow_id);
        attr_writer_uint16(flow_id);

        attr_reader_uint16(frag_len);
        attr_writer_uint16(frag_len);

        attr_reader_uint16(pack_len);
        attr_writer_uint16(pack_len);

        attr_reader_uint32(tstamp_lo);
        attr_writer_uint32(tstamp_lo);

        attr_reader_uint32(tstamp_hi);
        attr_writer_uint32(tstamp_hi);

    private:
        np_packet_hdr * _H_;
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const np_packet & h)
    {
        return out <<  "[flow_id="  << std::hex << static_cast<int>(h.flow_id()) << std::dec << 
                       " frag_len=" << static_cast<int>(h.frag_len()) << 
                       " pack_len=" << static_cast<int>(h.pack_len()) <<  
                       " stamp_lo=" << h.tstamp_lo() << 
                       " stamp_hi=" << h.tstamp_hi() << "]";

    }


} // namespace net 

#undef attr_reader
#undef attr_reader_uint16
#undef attr_reader_uint32
#undef attr_writer
#undef attr_writer_uint16
#undef attr_writer_uint32

#endif /* _NET_HEADERS_HH_ */
