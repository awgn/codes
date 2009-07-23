/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef SOCKADDRESS_HH
#define SOCKADDRESS_HH

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>

#include <enable_exception_if.hh>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

#ifndef ENABLE_EXCEPTION   
#define ENABLE_EXCEPTION true 
#endif

namespace more {

    template <int f> class sockaddress;
    template <>
    class sockaddress<AF_INET> : public more::enable_exception_if<ENABLE_EXCEPTION> {

        sockaddr_in _M_addr;
        socklen_t   _M_len;

    public:

        sockaddress() 
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in))
        {
            _M_addr.sin_family = AF_INET;
            _M_addr.sin_port   = 0;
            _M_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }

        sockaddress(const sockaddr_in &sa) 
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in))
        {
            assert(sa.sin_family == AF_INET);
            _M_addr.sin_family = AF_INET;
            _M_addr.sin_port   = sa.sin_port;
            _M_addr.sin_addr   = sa.sin_addr;
        }

        sockaddress(const std::string &host, unsigned short port ) 
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in))
        { 
            _M_addr.sin_family = AF_INET;
            _M_addr.sin_port   = htons(port);
            if (host.empty()) {
                _M_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                return;
            }

            struct addrinfo hints, * res;

            hints.ai_family   = AF_INET;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_flags    = 0;

            if (getaddrinfo(host.c_str(), NULL, &hints, &res) < 0) {
                throw_ ( std::runtime_error( std::string("getaddrinfo: ").append(gai_strerror(errno))) );
            }

            _M_addr.sin_addr = reinterpret_cast<struct sockaddr_in *>(res->ai_addr)->sin_addr;
            freeaddrinfo(res);
        }

        sockaddress(const sockaddress<AF_INET> &so)
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in))
        {
            _M_addr.sin_family = (&so)->sin_family;
            _M_addr.sin_addr   = (&so)->sin_addr;
            _M_addr.sin_port   = (&so)->sin_port;
        }

        const sockaddress &
        operator=(const sockaddress<AF_INET> &so)
        {
            if( & *this == &so )
                return *this;
            _M_addr.sin_family = (&so)->sin_family;
            _M_addr.sin_addr   = (&so)->sin_addr;
            _M_addr.sin_port   = (&so)->sin_port;
            return *this;
        }

        ~sockaddress()
        {}

        // set...

        void 
        host(const std::string &h) 
        {
            if (h.empty()) {
                _M_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                return;
            }

            struct addrinfo hints, * res;

            hints.ai_family   = AF_INET;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_flags    = 0;

            if (getaddrinfo(h.c_str(), NULL, &hints, &res) < 0) {
                throw_ ( std::runtime_error(std::string("getaddrinfo: ").append(gai_strerror(errno))) );
            }

            _M_addr.sin_addr = reinterpret_cast<struct sockaddr_in *>(res->ai_addr)->sin_addr;
            freeaddrinfo(res);
        }

        void 
        port(unsigned short port) 
        { _M_addr.sin_port = htons(port); }

        // get...
        const std::string 
        host() const 
        {
            char buf[64];
            if (inet_ntop(AF_INET, &_M_addr.sin_addr, buf, sizeof(buf)) <= 0) {
                throw_ ( std::runtime_error(std::string("inet_ntop: ").append(strerror(errno))), std::string() );
            }
            return std::string(buf);
        }

        unsigned short 
        port() const 
        { return ntohs(_M_addr.sin_port); }

        int 
        family() const 
        { return _M_addr.sin_family; }

        // taking address..
        sockaddr_in *
        operator &() 
        { return reinterpret_cast<struct sockaddr_in *>(&_M_addr); }

        const sockaddr_in *
        operator &() const 
        { return reinterpret_cast<const struct sockaddr_in *>(&_M_addr); }

        socklen_t &
        len() 
        { return _M_len; }

        const socklen_t &
        len() const 
        { return _M_len; }

    };

    template <>
    class sockaddress<AF_INET6> : public more::enable_exception_if<ENABLE_EXCEPTION> {

        sockaddr_in6 _M_addr;
        socklen_t    _M_len;

    public:

        sockaddress()
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in6))
        {
            _M_addr.sin6_family = AF_INET6;
            _M_addr.sin6_port   = 0;
            memcpy(&_M_addr.sin6_addr.s6_addr, &in6addr_any, sizeof(struct in6_addr));
        }

        sockaddress(const sockaddr_in6 &sa) 
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in6))
        {
            assert(sa.sin6_family == AF_INET6);
            _M_addr.sin6_family = AF_INET6;
            _M_addr.sin6_port   = sa.sin6_port;
            memcpy(_M_addr.sin6_addr.s6_addr, sa.sin6_addr.s6_addr, sizeof(struct in6_addr));
        }

        sockaddress(const std::string &host, unsigned short port ) 
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in6))
        {
            _M_addr.sin6_family = AF_INET6;
            _M_addr.sin6_port   = htons(port);
            if (host.empty()) {
                memcpy(&_M_addr.sin6_addr.s6_addr, &in6addr_any, sizeof(struct in6_addr));
                return;
            }
            if (inet_pton(AF_INET6, host.c_str(), &_M_addr.sin6_addr.s6_addr) <= 0) {
                throw_ ( std::runtime_error(std::string("inet_pton: ").append(strerror(errno))) );
            }
        }

        sockaddress(const sockaddress<AF_INET6> &so)
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_in6))
        {
            _M_addr.sin6_family = (&so)->sin6_family;
            _M_addr.sin6_port   = (&so)->sin6_port;
            memcpy(&_M_addr.sin6_addr.s6_addr, (&so)->sin6_addr.s6_addr, sizeof(struct in6_addr));
        }

        const sockaddress &
        operator=(const sockaddress<AF_INET6> &so)
        {
            if( & *this == &so )
                return *this;
            _M_addr.sin6_family = (&so)->sin6_family;
            _M_addr.sin6_port   = (&so)->sin6_port;
            memcpy(&_M_addr.sin6_addr.s6_addr, (&so)->sin6_addr.s6_addr, sizeof(struct in6_addr));
            return *this;
        }

        ~sockaddress()
        {}

        // set...
        void 
        host(const std::string &h) 
        {
            if (h.empty()) {
                memcpy(&_M_addr.sin6_addr.s6_addr, &in6addr_any, sizeof(struct in6_addr));
                return;
            }
            if (inet_pton(AF_INET6, h.c_str(), &_M_addr.sin6_addr.s6_addr) <= 0) {
                throw_ ( std::runtime_error(std::string("inet_pton: ").append(strerror(errno))) );
            }
        }

        void 
        port(unsigned short port) 
        { _M_addr.sin6_port = htons(port); }

        // get...
        const std::string 
        host() const 
        {
            char buf[64];
            if (inet_ntop(AF_INET6, &_M_addr.sin6_addr, buf, sizeof(buf)) <= 0) {
                throw_ ( std::runtime_error(std::string("inet_ntop: ").append(strerror(errno))), std::string() );
            }
            return std::string(buf);
        }

        unsigned short 
        port() const 
        { return ntohs(_M_addr.sin6_port); }

        int 
        family() const 
        { return _M_addr.sin6_family; }

        // taking address..
        struct sockaddr_in6 *
        operator &() 
        { return reinterpret_cast<struct sockaddr_in6 *>(&_M_addr); }

        const struct sockaddr_in6 *
        operator &() const
        { return reinterpret_cast<const struct sockaddr_in6 *>(&_M_addr); }

        socklen_t &
        len() 
        { return _M_len; }

        const socklen_t &
        len() const 
        { return _M_len; }

    };

    template <>
    class sockaddress<AF_UNIX> : public more::enable_exception_if<ENABLE_EXCEPTION> {

        sockaddr_un _M_addr;
        socklen_t _M_len;

    public:
        sockaddress()
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_un))
        {
            _M_addr.sun_family = AF_UNIX;
            _M_addr.sun_path[0]= '\0';
        }

        sockaddress(const std::string &name) 
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_un))
        {
            _M_addr.sun_family = AF_UNIX;
            strncpy(_M_addr.sun_path, name.c_str(), UNIX_PATH_MAX-1);
            _M_addr.sun_path[UNIX_PATH_MAX-1]='\0';
        }

        sockaddress(const sockaddress<AF_UNIX> &so)
        : _M_addr(),
          _M_len(sizeof(struct sockaddr_un))
        {
            _M_addr.sun_family = AF_UNIX;
            strncpy(_M_addr.sun_path, (&so)->sun_path, UNIX_PATH_MAX-1);
            _M_addr.sun_path[UNIX_PATH_MAX-1]='\0';
        }

        sockaddress &
        operator=(const sockaddress<AF_UNIX> &so)
        {
            if ( & *this == &so )
                return *this;
            _M_addr.sun_family = AF_UNIX;
            strncpy(_M_addr.sun_path, (&so)->sun_path, UNIX_PATH_MAX-1);
            _M_addr.sun_path[UNIX_PATH_MAX-1]='\0';
            return *this;
        }

        ~sockaddress()
        {}

        int 
        family() const 
        { return _M_addr.sun_family; }

        operator std::string() const 
        { return _M_addr.sun_path; }

        // taking address..
        sockaddr_un *
        operator &() 
        { return reinterpret_cast<sockaddr_un *>(&_M_addr); }

        const sockaddr_un *
        operator &() const 
        { return reinterpret_cast<const sockaddr_un *>(&_M_addr); }

        socklen_t &
        len()
        { return _M_len; }

        const socklen_t &
        len() const
        { return _M_len; }

    };

} // namespace more 

#endif /* SOCKADDRESS_HH */
