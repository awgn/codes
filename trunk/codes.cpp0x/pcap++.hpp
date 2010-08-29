/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_PCAP_HH_
#define _MORE_PCAP_HH_ 

#include <sys/types.h>
#include <arpa/inet.h>
#include <pcap.h>

#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <utility>
#include <list>
#include <tuple>      
#include <functional> 
 
#include <noncopyable.hpp>   // more!

namespace more { 

    ///////////////////////////////////////////////
    // pcap_sockaddr 
    ///////////////////////////////////////////////

    class pcap_sockaddr
    {
    public:
        explicit pcap_sockaddr(const sockaddr *s)
        : _M_storage()
        {
            if (s == NULL)
                return;

            if (s->sa_family == AF_INET)
                memcpy(&_M_storage, s, sizeof(struct sockaddr_in));

            if (s->sa_family == AF_INET6)
                memcpy(&_M_storage, s, sizeof(struct sockaddr_in6));
        }

        int
        family() const
        {
            return reinterpret_cast<const struct sockaddr &>(_M_storage).sa_family;
        }

        uint16_t 
        port() const
        {
            switch(this->family())
            {
            case AF_INET:
                return htons(reinterpret_cast<const struct sockaddr_in &>(_M_storage).sin_port);    
            case AF_INET6:
                return htons(reinterpret_cast<const struct sockaddr_in6 &>(_M_storage).sin6_port);    
            }
            return 0;
        }

        std::string
        addr() const
        {
            char buf[64] = { '\0' };

            switch(this->family())
            {
            case AF_INET: {
                if (inet_ntop(AF_INET, & reinterpret_cast<const struct sockaddr_in &>(_M_storage).sin_addr, buf, sizeof(buf)) <= 0 )
                    throw std::runtime_error("sockaddress::inet_ntop");
            } break;

            case AF_INET6: {
                if (inet_ntop(AF_INET6, & reinterpret_cast<const struct sockaddr_in6 &>(_M_storage).sin6_addr, buf, sizeof(buf)) <= 0 )
                    throw std::runtime_error("sockaddress::inet_ntop");
            } break;
            }
            
            return std::string(buf);
        }

    private:
        struct sockaddr_storage _M_storage;
    };

    ///////////////////////////////////////////////
    // pcap_addr structure
    ///////////////////////////////////////////////

    struct pcap_addr
    {        
        pcap_sockaddr addr;
        pcap_sockaddr netmask;
        pcap_sockaddr broadaddr;
        pcap_sockaddr dstaddr;    
 
        explicit pcap_addr(pcap_addr_t *p)
        : addr(p->addr),
          netmask(p->netmask),
          broadaddr(p->broadaddr),
          dstaddr(p->dstaddr)
        {} 
   };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const pcap_addr &a)
    {
        out << '[';

        if ( a.addr.family() == AF_INET || a.addr.family() == AF_INET6 )
            out << a.addr.addr() << '/' << a.netmask.addr() << ' ' << a.broadaddr.addr();

        if ( a.dstaddr.family())
          out << " p-t-p:" << a.dstaddr.addr();

        return out << ']';  
    }

    ///////////////////////////////////////////////
    // pcap_if structure
    ///////////////////////////////////////////////

    struct pcap_if
    {
        std::string name;
        std::string description;
        std::list<pcap_addr> addresses;
        int flags;   

        explicit pcap_if(pcap_if_t * i)
        : name(i->name),
          description(i->description ? : ""),
          flags(i->flags)
        {
            for(pcap_addr_t * addr = i->addresses; addr; addr = addr->next) {
                addresses.push_back(pcap_addr(addr));
            }
        }
    };

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const pcap_if & i)
    {
        out << "pcap_if[ name:" << i.name << " descr:'" << i.description << "' flags:" << i.flags << " "; 
        std::copy(i.addresses.begin(), i.addresses.end(), std::ostream_iterator<pcap_addr>(out, ","));
        return out << " ]";
    }

    ///////////////////////////////////////////////
    // bfp program class
    ///////////////////////////////////////////////

    class bpf_prog
    {
    public:
        bpf_prog(const std::string &str, bool optimize = true, bpf_u_int32 netmask = 0)
        : _M_prog(), 
          _M_str(str),
          _M_opt(optimize),
          _M_netmask(netmask)
        {
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
        }
    
        void 
        operator()(pcap_t *p)  // the program is to be compiled by the pcap class...
        {
            if ( pcap_compile(p, &_M_prog, _M_str.c_str(), _M_opt, _M_netmask) == -1)
               throw std::runtime_error( std::string("pcap: ").append(pcap_geterr(p))); 
        }

        ~bpf_prog()
        {
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
            pcap_freecode(&_M_prog);
        }

        operator bpf_program &()
        { 
            return _M_prog;
        }

        operator const bpf_program &() const
        {
            return  _M_prog;
        }

    private:
        struct bpf_program _M_prog;

        const std::string _M_str;
        bool _M_opt;
        bpf_u_int32 _M_netmask;
    };

    ///////////////////////////////////////////////
    ///////////////////////////////////////////////
    // pcap class
    ///////////////////////////////////////////////
    ///////////////////////////////////////////////

    class pcap_dumper;
    class pcap : private noncopyable
    {

    protected:
        friend class pcap_dumper;

        virtual ~pcap()
        {
            pcap_close(_M_handle);
        }

        pcap()
        : _M_errbuf(),
          _M_handle()
        {} 

        /////////////////////////////////////////////////////////////////////////
        // get pcap_t * handle...
 
        operator pcap_t *()
        { 
            return _M_handle;
        }

        operator const pcap_t *() const
        {
            return  _M_handle;
        }

    public:

        virtual 
        void packet_handler(const struct pcap_pkthdr *, const u_char *)
        {
            throw std::runtime_error("virtual handler not implemented!");
        }

        static
        void static_handler(u_char *that, const struct pcap_pkthdr *h, const u_char *bytes)
        {
            reinterpret_cast<pcap *>(that)->packet_handler(h,bytes);
        }

        /////////////////////////////////////////////////////////////////////////

        void
        nonblock(bool value)
        {
            clear_errbuf();

            if (pcap_setnonblock(_M_handle, value, _M_errbuf) == -1)
                throw std::runtime_error(std::string("pcap: ").append(_M_errbuf)); 
        }

        bool 
        nonblock() const
        {
             clear_errbuf();

             int value;
             if ((value = pcap_getnonblock(_M_handle, _M_errbuf)) == -1)
                throw std::runtime_error(std::string("pcap: ").append(_M_errbuf)); 
             
             return value;  
        }

        virtual const std::string
        device() const
        { 
            return "pcap::device";
        }

        /////////////////////////////////////////////////////////////////////////

        int dispatch(int cnt)
        {
            int n;
            if (  (n = pcap_dispatch(_M_handle, cnt, static_handler, reinterpret_cast<u_char *>(this))) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));
            return n;
        }

        // direct version... 
        int dispatch(int cnt, pcap_handler h, u_char *u = 0)
        {
            int n;
            if (  (n = pcap_dispatch(_M_handle, cnt, h, u)) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));
            return n;
        }

        int loop(int cnt)
        {
            int n;
            if (  (n = pcap_loop(_M_handle, cnt, static_handler, reinterpret_cast<u_char *>(this))) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));
            return n;
        }

        // direct version...
        int loop(int cnt, pcap_handler h, u_char *u = 0)
        {
            int n;
            if (  (n = pcap_loop(_M_handle, cnt, h, u)) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));
            return n;
        }

        const u_char *
        next(struct pcap_pkthdr *h)
        {
            // Unfortunately, there is no way to determine whether an error occured or not.
            // NULL is returned if no packers were read from a live capture, or if no more packets
            // are available in a ``savefile``.

            return pcap_next(_M_handle, h); 
        }
       
        int next_ex(struct pcap_pkthdr **pkt_header, const u_char **pkt_data)
        {
            int n;
            if( (n=pcap_next_ex(_M_handle, pkt_header, pkt_data)) == -1 )
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));
            return n;
        }

        void breakloop()
        {
            pcap_breakloop(_M_handle);
        }
        
        /////////////////////////////////////////////////////////////////////////
        // filters...

        void filter(bpf_prog & prog)
        {
            // compile the bpf_prog first.
            //
            prog(_M_handle);

            if ( pcap_setfilter(_M_handle, & static_cast<bpf_program &>(prog)) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));
                
        }

        void direction(pcap_direction_t dir)
        {
            if ( pcap_setdirection(_M_handle, dir) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));
        } 

        /////////////////////////////////////////////////////////////////////////
    
        int inject(const void *buf, size_t size)
        {
            int r;
            if ( ( r = pcap_inject(_M_handle, buf,size) ) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle)));

            return r;
        }

        ///////////////////////////////////////////////
        // get errors... 

        std::string
        geterr() const
        {
            return pcap_geterr(_M_handle);
        }

        std::string
        errbuf() const
        {
            return std::string(_M_errbuf);
        }

        ///////////////////////////////////////////////
        // static methods... pcap utils
        //

        static
        std::string lookupdev()
        {
            char errbuf[PCAP_ERRBUF_SIZE];
            char *dev = pcap_lookupdev(errbuf);
            if (dev == NULL)
                throw std::runtime_error(std::string("pcap: ").append(errbuf)); 
            
            return std::string(dev);
        }
    
        static
        std::pair<bpf_u_int32, bpf_u_int32> 
        lookupnet(const std::string &dev)
        {
            char errbuf[PCAP_ERRBUF_SIZE];
            bpf_u_int32 net, mask;
            if (pcap_lookupnet(dev.c_str(), &net, &mask, errbuf) == -1)
                throw std::runtime_error(std::string("pcap: ").append(errbuf)); 
        
            return std::make_pair(net,mask);
        }

        static
        std::string
        ipv4_dotform(bpf_u_int32 value)
        {
            char buf[16];
            if (inet_ntop(AF_INET, &value, buf, sizeof(buf)) <= 0 )
                throw std::runtime_error("sockaddress::inet_ntop");

            return std::string(buf);
        }

        static
        std::list<pcap_if> 
        findalldevs()
        {
            char errbuf[PCAP_ERRBUF_SIZE];
            pcap_if_t * all, * dev;
            if ( pcap_findalldevs(&all, errbuf) == -1 )
                throw std::runtime_error(std::string("pcap: ").append(errbuf)); 

            std::list<pcap_if> ret;
            for( dev=all ; dev; dev = dev->next)
                ret.push_back(pcap_if(dev));

            pcap_freealldevs(all);
            return ret;
        }

        static
        const char * 
        lib_version()
        {
            return pcap_lib_version();
        }

        ///////////////////////////////////////////////
        // datalink...

        static
        int datalink_name_to_val(const char *name)
        {
            return pcap_datalink_name_to_val(name);
        }

        static 
        const char *
        datalink_val_to_name(int dlt)
        {
            return pcap_datalink_val_to_name(dlt);
        }

        static 
        const char *
        datalink_val_to_description(int dlt)
        {
            return pcap_datalink_val_to_description(dlt);
        }

        int datalink() const
        {
            return pcap_datalink(const_cast<pcap_t *>(_M_handle));
        }

        size_t
        datalink_len(int dtl = -1) const
        {
            if (dtl == -1)
                dtl = this->datalink();

            switch (dtl) {
                case DLT_NULL: return 4;
                case DLT_EN10MB: return 14;
                case DLT_EN3MB: return 14;
                // case DLT_AX25: return -1;
                // case DLT_PRONET: return -1;
                // case DLT_CHAOS: return -1;
                case DLT_IEEE802: return 22;
                // case DLT_ARCNET: return -1;
#if defined (__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__) || defined (__BSDI__)
                case DLT_SLIP: return 16;
#else
                case DLT_SLIP: return 24;
#endif

#if defined (__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__)
                case DLT_PPP: return 4;
#elif defined (__sun)
                case DLT_PPP: return 8;
#else
                case DLT_PPP: return 24;
#endif
                case DLT_FDDI: return 21;
                case DLT_ATM_RFC1483: return 8;

                case DLT_LOOP: return 4;	// according to OpenBSD DLT_LOOP
                                            // collision: see "bpf.h"
                case DLT_RAW: return 0;

                case DLT_SLIP_BSDOS: return 16;
                case DLT_PPP_BSDOS: return 4;
                // case DLT_ATM_CLIP: return -1;
#if defined (__FreeBSD__) || defined (__OpenBSD__) || defined (__NetBSD__)
                case DLT_PPP_SERIAL: return 4;
                case DLT_PPP_ETHER: return 4;
#elif defined (__sun)
                case DLT_PPP_SERIAL: return 8;
                case DLT_PPP_ETHER: return 8;
#else
                case DLT_PPP_SERIAL: return 24;
                case DLT_PPP_ETHER: return 24;
#endif
                // case DLT_C_HDLC: return -1;
                case DLT_IEEE802_11: return 30;
                case DLT_LINUX_SLL: return 16;
                // case DLT_LTALK: return -1;
                // case DLT_ECONET: return -1;
                // case DLT_IPFILTER: return -1;
                // case DLT_PFLOG: return -1;
                // case DLT_CISCO_IOS: return -1;
                // case DLT_PRISM_HEADER: return -1;
                // case DLT_AIRONET_HEADER: return -1;
            }
            throw std::runtime_error("pcap: unknown datalink type");
            return -1;
        }

        void 
        set_datalink(int dlt)
        {
            if ( pcap_set_datalink(_M_handle, dlt) == -1 )
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle))); 
        }

    protected:
        void 
        clear_errbuf() const
        {
            _M_errbuf[0]='\0';
        }

        bool 
        warning() const
        {
            return strlen(_M_errbuf) > 0;
        } 

        mutable char _M_errbuf[PCAP_ERRBUF_SIZE];
        pcap_t * _M_handle;
    };

    ///////////////////////////////////////////////
    // pcap_live class
    ///////////////////////////////////////////////

    class pcap_live : public pcap
    {   
    public:     
        pcap_live(const std::string &dev, int snaplen, bool promisc, int to_ms)
        : pcap(),
          _M_device(dev),
          _M_snaplen(snaplen),
          _M_promisc(promisc),
          _M_to_ms(to_ms)
        {
            this->clear_errbuf();

            _M_handle = pcap_open_live(_M_device.c_str(), _M_snaplen, _M_promisc, _M_to_ms, _M_errbuf);
            if (_M_handle == NULL)
                throw std::runtime_error(std::string("pcap: ").append(_M_errbuf));

            if ( this->warning() )
                std::clog << "pcap warning: " << _M_errbuf << std::endl;
        }
        
        const std::string
        device() const
        { 
            return _M_device;
        }

        int 
        snaplen() const
        {
            return _M_snaplen;
        }

        bool
        promisc() const
        { 
            return _M_promisc;
        }

        int 
        timeo_ms() const
        {
            return _M_to_ms;
        }

        ///////////////////////////////////////////////

        void
        stats(struct pcap_stat *ps)
        {
            if (pcap_stats(_M_handle, ps) == -1)
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(_M_handle))); 
        }

    private:
        std::string _M_device;
        int  _M_snaplen;
        bool _M_promisc;
        int  _M_to_ms;
    };


    ///////////////////////////////////////////////
    // pcap_dead class
    ///////////////////////////////////////////////

    class pcap_dead : public pcap
    {   
    public:            
        pcap_dead(int linktype, int snaplen)
        : pcap(),
          _M_linktype(linktype),
          _M_snaplen(snaplen)
        {
            this->clear_errbuf();

            _M_handle = pcap_open_dead(linktype, _M_snaplen);
            if (_M_handle == NULL)
                throw std::runtime_error(std::string("pcap: ").append(_M_errbuf));

            if ( this->warning() )
                std::clog << "pcap warning: " << _M_errbuf << std::endl;
        } 

        int 
        snaplen() const
        {
            return _M_snaplen;
        }

        int 
        linktype() const
        {
            return _M_linktype;
        }

    private:
        int  _M_linktype;
        int  _M_snaplen;
    };

    ///////////////////////////////////////////////
    // pcap_offline class (read pcap file)
    ///////////////////////////////////////////////

    class pcap_offline : public pcap
    {   
    public:          
        explicit pcap_offline(const std::string &fname)        
        : pcap(),
          _M_device(fname)
        {
            clear_errbuf();
            _M_handle = pcap_open_offline(_M_device.c_str(), _M_errbuf);
            if ( _M_handle == NULL )
                throw std::runtime_error(std::string("pcap: ").append(_M_errbuf));
 
            if ( this->warning() )
                std::clog << "pcap warning: " << _M_errbuf << std::endl;
        }

        const std::string
        device() const
        { 
            return _M_device;
        }

        ///////////////////////////////////////////////
       
        int 
        major_version() const
        {
            return pcap_major_version(_M_handle); 
        }

        int 
        minor_version() const
        {
            return pcap_minor_version(_M_handle); 
        }

        bool
        is_swapped() const
        { 
            return pcap_is_swapped(_M_handle);
        }

    private:
        std::string _M_device;
    };

    ///////////////////////////////////////////////
    // pcap_dumper class (write to a pcap file)
    ///////////////////////////////////////////////

    class pcap_dumper
    {
    public:
        pcap_dumper(pcap & source, const char *fname)
        : _M_dumper(0)
        {
            pcap_t * h = static_cast<pcap_t *>(source);
            if ( (_M_dumper = pcap_dump_open(h, fname)) == NULL )
                throw std::runtime_error(std::string("pcap: ").append(pcap_geterr(h)));
        }

        ~pcap_dumper()
        {
            pcap_dump_close(_M_dumper);
        }

        void
        dump(const struct pcap_pkthdr *h, const u_char *sp)
        {
            pcap_dump(reinterpret_cast<u_char *>(_M_dumper), h, sp);
        }

        void
        flush()
        {
            if ( pcap_dump_flush(_M_dumper) == -1 )
                throw std::runtime_error(std::string("pcap: pcap_dump_flush")); 
        }

        static
        void handler(u_char *user, const struct pcap_pkthdr *h, const u_char *sp)
        {
            pcap_dumper * that = reinterpret_cast<pcap_dumper *>(user);
            that->dump(h,sp);
        }

    private:
        pcap_dumper_t * _M_dumper;
    };

} // namespace more

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &out, const pcap_pkthdr & h)
    {
        return out << '[' << h.ts.tv_sec << ':' << h.ts.tv_usec << " caplen:" << h.caplen << " len:" << h.len << ']';
    }

#endif /* _MORE_PCAP_HH_ */
