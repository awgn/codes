/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _URL_HH_
#define _URL_HH_ 

#include <netdb.h>

#include <lexical_cast.hh>  // more!

#include <stdexcept>
#include <iostream>
#include <string>

namespace more {

    class url
    {
    public:

        url()
        : m_protocol(),
          m_user(),
          m_pass(),
          m_host(),
          m_path(),
          m_port(0)
        {}

        url(const std::string &s)
        {
            std::string::size_type p = s.find("://");
            if (p == std::string::npos)
                throw std::runtime_error("url: missing protocol");    
            
            m_protocol.assign(s.c_str(),s.c_str()+p);

            std::string baseurl;

            std::string::size_type e = s.find("/", p+3);
            if (e == std::string::npos) {
                baseurl.assign(s.c_str()+p+3);     
            } else {
                baseurl.assign(s.c_str()+p+3, s.c_str() + e);
                m_path.assign(s.c_str() + e + 1);     
            }

            std::string::size_type at = baseurl.find("@"); 
            if (at != std::string::npos) {
                // parsing user and pass...
                std::string::size_type x = baseurl.find(":");
                if ( x == std::string::npos ) {
                    m_user.assign(baseurl.c_str(), baseurl.c_str()+at);
                } else {
                    m_user.assign(baseurl.c_str(), baseurl.c_str()+x);
                    m_pass.assign(baseurl.c_str()+x+1, baseurl.c_str()+at);
                }

                p = at + 1;
            } else {
                p = 0;
            }

            // parsing host and port...
            std::string::size_type c = baseurl.find(":", p);
            if ( c != std::string::npos ) {
                m_host.assign(baseurl.c_str()+p, baseurl.c_str()+c);
                m_port = more::lexical_cast<unsigned short>(std::string(baseurl.c_str()+c+1));

            } else {
                m_host.assign(baseurl.c_str()+p);
                
                struct servent * s = getservbyname(m_protocol.c_str(),NULL);
                if ( s == NULL )
                    throw std::runtime_error("url: unknown protocol");    

                m_port = htons(s->s_port);
                endservent();
            }
        }

        ~url()
        {}

        const std::string &
        proto() const
        { return m_protocol; }

        const std::string &
        user() const
        { return m_user; }

        const std::string &
        pass() const
        { return m_pass; }

        const std::string &
        host() const
        { return m_host; }

        const std::string &
        path() const
        { return m_path; }

        unsigned short
        port() const
        { return m_port; }

        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT,Traits> &out, const url & obj)
        {
            out << obj.m_protocol << "://";
            if (!obj.m_user.empty()) {
                out << obj.m_user; 
                if (!obj.m_pass.empty()) 
                    out << ':' << obj.m_pass;
                out << '@';
            }
            out << obj.m_host << ':' << obj.m_port;
            if (!obj.m_path.empty())
                out << '/' << obj.m_path;

            return out;
        }

    private:

        std::string m_protocol;
        std::string m_user;
        std::string m_pass;
        std::string m_host;
        std::string m_path;
        unsigned short m_port;
    };
}

#endif /* _URL_HH_ */
