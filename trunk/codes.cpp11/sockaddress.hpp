/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SOCKADDRESS_HPP_
#define _SOCKADDRESS_HPP_ 

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>
#include <string>
#include <cassert>
#include <stdexcept>
#include <system_error>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

namespace more {

    template <int f> class sockaddress;

    template <>
    class sockaddress<AF_INET> 
    {
        sockaddr_in m_addr;
        socklen_t   m_len;

    public:

        sockaddress() 
        : m_addr()
        , m_len(sizeof(sockaddr_in))
        {
            m_addr.sin_family = AF_INET;
            m_addr.sin_port   = 0;
            m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        }

        sockaddress(const sockaddr_in &sa) 
        : m_addr()
        , m_len(sizeof(sockaddr_in))
        {
            assert(sa.sin_family == AF_INET);
            m_addr.sin_family = AF_INET;
            m_addr.sin_port   = sa.sin_port;
            m_addr.sin_addr   = sa.sin_addr;
        }

        sockaddress(const in_addr &addr, unsigned short port) 
        : m_addr()
        , m_len(sizeof(sockaddr_in))
        {
            m_addr.sin_family = AF_INET;
            m_addr.sin_addr   = addr;
            m_addr.sin_port   = htons(port);
        }

        sockaddress(const std::string &host, unsigned short port) 
        : m_addr()
        , m_len(sizeof(sockaddr_in))
        { 
            m_addr.sin_family = AF_INET;
            m_addr.sin_port   = htons(port);
            if (host.empty()) {
                m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                return;
            }

            addrinfo hints, * res;

            hints.ai_family   = AF_INET;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_flags    = 0;

            if (getaddrinfo(host.c_str(), nullptr, &hints, &res) < 0) 
            {
                throw std::system_error(errno, std::generic_category());
            }

            m_addr.sin_addr = reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr;
            freeaddrinfo(res);
        }

        sockaddress(sockaddress const &so)
        : m_addr()
        , m_len(sizeof(sockaddr_in))
        {
            m_addr.sin_family = so.m_addr.sin_family;
            m_addr.sin_addr   = so.m_addr.sin_addr;
            m_addr.sin_port   = so.m_addr.sin_port;
        }

        const sockaddress &
        operator=(sockaddress const &so)
        {
            if(this == &so)
                return *this;

            m_addr.sin_family = so.m_addr.sin_family;
            m_addr.sin_addr   = so.m_addr.sin_addr;
            m_addr.sin_port   = so.m_addr.sin_port;
            return *this;
        }


        ~sockaddress() = default;

        // set...

        void 
        host(const std::string &h) 
        {
            if (h.empty()) {
                m_addr.sin_addr.s_addr = htonl(INADDR_ANY);
                return;
            }

            addrinfo hints, * res;

            hints.ai_family   = AF_INET;
            hints.ai_socktype = 0;
            hints.ai_protocol = 0;
            hints.ai_flags    = 0;

            if (getaddrinfo(h.c_str(), nullptr, &hints, &res) < 0) 
            {
                throw std::system_error(errno, std::generic_category());
            }

            m_addr.sin_addr = reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr;
            freeaddrinfo(res);
        }

        void 
        port(unsigned short port) 
        { 
            m_addr.sin_port = htons(port); 
        }

        // get...
        
        const std::string 
        host() const 
        {
            char buf[64];
            if (inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf)) == nullptr) 
            {
                throw std::system_error(errno, std::generic_category());
            }
            return std::string(buf);
        }

        unsigned short 
        port() const 
        { 
            return ntohs(m_addr.sin_port); 
        }

        int 
        family() const 
        { 
            return m_addr.sin_family; 
        }

        // taking address..
        
        sockaddr &
        c_addr() 
        { 
            return reinterpret_cast<sockaddr &>(m_addr); 
        }

        const sockaddr &
        c_addr() const 
        { 
            return reinterpret_cast<const sockaddr &>(m_addr); 
        }

        socklen_t &
        len() 
        { 
            return m_len; 
        }

        const socklen_t &
        len() const 
        { 
            return m_len; 
        }

    };

    template <>
    class sockaddress<AF_INET6> {

        sockaddr_in6 m_addr;
        socklen_t    m_len;

    public:
        sockaddress()
        : m_addr()
        , m_len(sizeof(sockaddr_in6))
        {
            m_addr.sin6_family = AF_INET6;
            m_addr.sin6_port   = 0;
            memcpy(&m_addr.sin6_addr.s6_addr, &in6addr_any, sizeof(in6_addr));
        }

        sockaddress(const sockaddr_in6 &sa) 
        : m_addr()
        , m_len(sizeof(sockaddr_in6))
        {
            assert(sa.sin6_family == AF_INET6);
            m_addr.sin6_family = AF_INET6;
            m_addr.sin6_port   = sa.sin6_port;
            memcpy(m_addr.sin6_addr.s6_addr, sa.sin6_addr.s6_addr, sizeof(in6_addr));
        }
        
        sockaddress(const in6_addr &addr, unsigned short port) 
        : m_addr()
        , m_len(sizeof(sockaddr_in))
        {
            m_addr.sin6_family = AF_INET6;
            m_addr.sin6_port   = htons(port);
            memcpy(m_addr.sin6_addr.s6_addr, addr.s6_addr, sizeof(in6_addr));
        }

        sockaddress(const std::string &host, unsigned short port ) 
        : m_addr()
        , m_len(sizeof(sockaddr_in6))
        {
            m_addr.sin6_family = AF_INET6;
            m_addr.sin6_port   = htons(port);
            if (host.empty()) {
                memcpy(&m_addr.sin6_addr.s6_addr, &in6addr_any, sizeof(in6_addr));
                return;
            }
            if (inet_pton(AF_INET6, host.c_str(), &m_addr.sin6_addr.s6_addr) <= 0) {
                throw std::system_error(errno, std::generic_category());
            }
        }

        sockaddress(sockaddress const &so)
        : m_addr()
        , m_len(sizeof(sockaddr_in6))
        {
            m_addr.sin6_family = so.m_addr.sin6_family;
            m_addr.sin6_port   = so.m_addr.sin6_port;
            memcpy(&m_addr.sin6_addr.s6_addr, so.m_addr.sin6_addr.s6_addr, sizeof(in6_addr));
        }

        const sockaddress &
        operator=(sockaddress const &so)
        {
            if(this == &so)
                return *this;
            m_addr.sin6_family = so.m_addr.sin6_family;
            m_addr.sin6_port   = so.m_addr.sin6_port;
            memcpy(&m_addr.sin6_addr.s6_addr, so.m_addr.sin6_addr.s6_addr, sizeof(in6_addr));
            return *this;
        }


        ~sockaddress() = default;

        // set...

        void 
        host(const std::string &h) 
        {
            if (h.empty()) {
                memcpy(&m_addr.sin6_addr.s6_addr, &in6addr_any, sizeof(in6_addr));
                return;
            }
            if (inet_pton(AF_INET6, h.c_str(), &m_addr.sin6_addr.s6_addr) <= 0) {
                throw std::system_error(errno, std::generic_category());
            }
        }

        void 
        port(unsigned short port) 
        { 
            m_addr.sin6_port = htons(port); 
        }

        // get...
        
        const std::string 
        host() const 
        {
            char buf[64];
            if (inet_ntop(AF_INET6, &m_addr.sin6_addr, buf, sizeof(buf)) == nullptr) {
                throw std::system_error(errno, std::generic_category());
            }
            return std::string(buf);
        }

        unsigned short 
        port() const 
        { 
            return ntohs(m_addr.sin6_port); 
        }

        int 
        family() const 
        { 
            return m_addr.sin6_family; 
        }

        // taking address..
        
        sockaddr &
        c_addr()
        { 
            return reinterpret_cast<sockaddr &>(m_addr); 
        }

        const sockaddr &
        c_addr() const
        { 
            return reinterpret_cast<const sockaddr &>(m_addr); 
        }

        socklen_t &
        len() 
        { 
            return m_len; 
        }

        const socklen_t &
        len() const 
        {
            return m_len; 
        }

    };

    template <>
    class sockaddress<AF_UNIX> {

        sockaddr_un m_addr;
        socklen_t m_len;

    public:
        sockaddress()
        : m_addr()
        , m_len(sizeof(sockaddr_un))
        {
            m_addr.sun_family = AF_UNIX;
            m_addr.sun_path[0]= '\0';
        }

        sockaddress(const std::string &name) 
        : m_addr()
        , m_len(sizeof(sockaddr_un))
        {
            m_addr.sun_family = AF_UNIX;
            strncpy(m_addr.sun_path, name.c_str(), UNIX_PATH_MAX-1);
            m_addr.sun_path[UNIX_PATH_MAX-1]='\0';
        }

        sockaddress(sockaddress const &so)
        : m_addr()
        , m_len(sizeof(sockaddr_un))
        {
            m_addr.sun_family = AF_UNIX;
            strncpy(m_addr.sun_path, so.m_addr.sun_path, UNIX_PATH_MAX-1);
            m_addr.sun_path[UNIX_PATH_MAX-1]='\0';
        }

        sockaddress &
        operator=(sockaddress const &so)
        {
            if (this == &so)
                return *this;
            m_addr.sun_family = AF_UNIX;
            strncpy(m_addr.sun_path, so.m_addr.sun_path, UNIX_PATH_MAX-1);
            m_addr.sun_path[UNIX_PATH_MAX-1]='\0';
            return *this;
        }

        ~sockaddress() = default;

        int 
        family() const 
        { 
            return m_addr.sun_family; 
        }

        std::string
        name() const 
        { 
            return m_addr.sun_path; 
        }

        // taking address..
        sockaddr &
        c_addr() 
        { 
            return reinterpret_cast<sockaddr &>(m_addr); 
        }

        const sockaddr &
        c_addr() const 
        { 
            return reinterpret_cast<const sockaddr &>(m_addr); 
        }

        socklen_t &
        len()
        { 
            return m_len; 
        }

        const socklen_t &
        len() const
        { 
            return m_len; 
        }

    };

} // namespace more 

#endif /* _SOCKADDRESS_HPP_ */
