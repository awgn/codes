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
#include <stdexcept>

#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>

#include <net/ethernet.h>
#include <arpa/inet.h>

namespace more {

    namespace header_helper {

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
        void ctor(P * &p, ssize_t &bytes, ssize_t size, header_helper::int2type<N>)
        {
            if (bytes < N)
                throw std::range_error("T::static_size");
            
            if ( size  && size != N )
                throw std::range_error("size != T::static_size");

            p = reinterpret_cast<P *>(reinterpret_cast<char *>(p) + N);
            bytes -= N;
        }

        ///////////////////////////////////////////
        // ctor for dynamic size headers.. ie: ip 

        template <typename P>
        void ctor(P * &p, ssize_t &bytes, ssize_t size, header_helper::int2type<0>)
        {
            
            ssize_t n = size ? _M_value.size(bytes, size) :
                               _M_value.size(bytes);
            if (bytes < n)
                throw std::range_error("T::size() [dynamic size]");
            p = reinterpret_cast<P *>(reinterpret_cast<char *>(p) + n);
            bytes -= n;
        }

    public:
        template <typename P>
        header(P * &ptr, ssize_t &bytes, ssize_t size = 0 /* != 0 means force the size */ )
        : _M_value( const_cast< 
                        typename header_helper::remove_const_if< 
                            std::tr1::is_const<T>::value, P
                            >::type * >(ptr))
        {
           ctor(ptr, bytes, size, header_helper::int2type<T::static_size>());
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

    struct update {};
    struct verify {};

    static inline unsigned short
    in_chksum(uint16_t * addr, int len)
    {
        register int    nleft = len;
        register int    sum = 0;
        uint16_t        answer = 0;

        while (nleft > 1) {
            sum += *addr++;
            nleft -= 2;
        }

        if (nleft == 1) {
            *(u_char *) (&answer) = *(u_char *) addr;
            sum += answer;
        }
        sum = (sum >> 16) + (sum + 0xffff);
        sum += (sum >> 16);
        answer = ~sum;
        return (answer);
    }

    //////////////////////////////////////////////////////////
    // ethernet header

    class ethernet
    {
    public:
        static const int static_size = sizeof(ether_header);
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
                       " type=0x" << std::hex << h.ether_type() << std::dec <<  "]";
    }

    class ipv4
    {
    public:
        static const int static_size = 0;   // dynamic size
        friend class more::header<ipv4>;

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
        //////////////////////////////////////////////////////

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
        check(net::update)
        {
            _H_->check = 0;
            _H_->check = in_chksum((uint16_t *)_H_, this->size()); 
        }

        bool
        check(net::verify)
        {
            return in_chksum((uint16_t *)_H_, this->size()) == 0;
        }

        std::string
        saddr() const
        {
            char buf[16];
            inet_ntop(AF_INET, &_H_->saddr, buf, sizeof(buf));
            return std::string(buf);
        }
        
        void
        saddr(const std::string &ip_addr)
        {
            if (inet_pton(AF_INET,ip_addr.c_str(), &_H_->saddr) <= 0)
               throw std::runtime_error("ipv4::saddr:  inet_pton");
        }

        std::string
        daddr() const
        {            
            char buf[16];
            inet_ntop(AF_INET, &_H_->daddr, buf, sizeof(buf));
            return std::string(buf);
        }
        
        void
        daddr(const std::string &ip_addr)
        {
            if (inet_pton(AF_INET,ip_addr.c_str(), &_H_->daddr) <= 0)
               throw std::runtime_error("ipv4::saddr:  inet_pton");
        }

    private:
        iphdr * _H_;
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const ipv4 & h)
    {
        return out << std::hex << 
            "[ihl=" << h.ihl() << 
            " ver:" << h.version() <<
            " tos=0x" << static_cast<uint16_t>(h.tos()) << std::dec << 
            " tot_len=" << h.tot_len() << std::hex << 
            " id=0x"  << h.id() << std::dec << 
            " frag_off=" << h.frag_off() <<
            " ttl=" << static_cast<uint16_t>(h.ttl()) << std::hex << 
            " proto=" << static_cast<uint16_t>(h.protocol()) << 
            " checksum=" << h.check() <<
            " saddr=" << h.saddr() << 
            " daddr=" << h.daddr() << "]" << std::dec;
    }

    class udp
    {
    public:
        static const int static_size = sizeof(udphdr);   // static size
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
                       " dest=" << h.dest() << 
                       " len=" << h.len() <<  
                       " check=0x" << std::hex << h.check() << std::dec << "]";
    }

    class tcp
    {
    public:
        static const int static_size = 0;   // dynamic size
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
        //////////////////////////////////////////////////////

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

        attr_reader_uint16(window);
        attr_writer_uint16(window);

        attr_reader_uint16(check);
        attr_writer_uint16(check);

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
                       " flags=" << 
                                    ( h.cwr() ? "W" : "" ) <<
                                    ( h.ece() ? "E" : "" ) <<
                                    ( h.urg() ? "U" : "" ) <<
                                    ( h.ack() ? "A" : "" ) <<
                                    ( h.psh() ? "P" : "" ) <<
                                    ( h.rst() ? "R" : "" ) <<
                                    ( h.syn() ? "S" : "" ) <<
                                    ( h.fin() ? "F" : "" ) <<

                       " window=" << h.window() << 
                       " check=0x" << std::hex << h.check() << std::dec << 
                       " urg_ptr=" << h.urg_ptr() << "]";
    }


} // namespace net 

#undef attr_reader
#undef attr_reader_uint16
#undef attr_reader_uint32
#undef attr_writer
#undef attr_writer_uint16
#undef attr_writer_uint32

#endif /* _NET_HEADERS_HH_ */
