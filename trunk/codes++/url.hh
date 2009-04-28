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

#include <stdexcept>
#include <iostream>
#include <string>
#include <lexical_cast.hh>

#include <netdb.h>

namespace more {

    class url
    {
    public:

        url()
        : _M_protocol(),
          _M_user(),
          _M_pass(),
          _M_host(),
          _M_path(),
          _M_port(0)
        {}

        url(const std::string &s)
        {
            std::string::size_type p = s.find("://");
            if (p == std::string::npos)
                throw std::runtime_error("url: missing protocol");    
            
            _M_protocol.assign(s.c_str(),s.c_str()+p);

            std::string baseurl;

            std::string::size_type e = s.find("/", p+3);
            if (e == std::string::npos) {
                baseurl.assign(s.c_str()+p+3);     
            }
            else {
                baseurl.assign(s.c_str()+p+3, s.c_str() + e);
                _M_path.assign(s.c_str() + e + 1);     
            }

            std::string::size_type at = baseurl.find("@"); 
            if (at != std::string::npos) {
                // parsing user and pass...
                std::string::size_type x = baseurl.find(":");
                if ( x == std::string::npos ) {
                    _M_user.assign(baseurl.c_str(), baseurl.c_str()+at);
                }
                else {
                    _M_user.assign(baseurl.c_str(), baseurl.c_str()+x);
                    _M_pass.assign(baseurl.c_str()+x+1, baseurl.c_str()+at);
                }

                p = at + 1;
            } else p = 0;

            // parsing host and port...
            std::string::size_type c = baseurl.find(":", p);
            if ( c != std::string::npos ) {
                _M_host.assign(baseurl.c_str()+p, baseurl.c_str()+c);
                _M_port = more::lexical_cast<unsigned short>(std::string(baseurl.c_str()+c+1));

            } else {
                _M_host.assign(baseurl.c_str()+p);
                
                struct servent * s = getservbyname(_M_protocol.c_str(),NULL);
                if ( s == NULL )
                    throw std::runtime_error("url: unknown protocol");    

                _M_port = htons(s->s_port);
                endservent();
            }
        }

        ~url()
        {}

        const std::string &
        proto() const
        { return _M_protocol; }

        const std::string &
        user() const
        { return _M_user; }

        const std::string &
        pass() const
        { return _M_pass; }

        const std::string &
        host() const
        { return _M_host; }

        const std::string &
        path() const
        { return _M_path; }

        unsigned short
        port() const
        { return _M_port; }

        friend 
        std::ostream & operator<<(std::ostream &out, const url & obj)
        {
            out << obj._M_protocol << "://";
            if (!obj._M_user.empty()) {
                out << obj._M_user; 
                if (!obj._M_pass.empty()) 
                    out << ':' << obj._M_pass;
                out << '@';
            }
            out << obj._M_host << ':' << obj._M_port;
            if (!obj._M_path.empty())
                out << '/' << obj._M_path;

            return out;
        }

    private:

        std::string _M_protocol;
        std::string _M_user;
        std::string _M_pass;
        std::string _M_host;
        std::string _M_path;
        unsigned short _M_port;
    };
}

#endif /* _URL_HH_ */
