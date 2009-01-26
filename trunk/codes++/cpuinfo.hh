/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef CPUINFO_HH
#define CPUINFO_HH

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <map>

namespace proc
{
    template <int CPU_MAX=16>
    class cpuinfo
    {
        struct adaptor 
        {
            std::string _M_value;

            adaptor(const std::string &x) 
            : _M_value(x) 
            {}

            operator std::string()
            {
                return _M_value;
            }
            operator int()
            {
                int r = ::strtol(_M_value.c_str(), NULL, 0);
                return r;
            }
            operator double()
            {
                double r = ::strtod(_M_value.c_str(), NULL);
                return r;
            }
            operator long double()
            {
                long double r = ::strtold(_M_value.c_str(), NULL);
                return r;
            }
        };

        typedef std::map<std::string, adaptor> cpu_map;

        cpu_map _M_map[CPU_MAX];
        int _M_processor;

        static std::string trim (const std::string &orig)
        {
            std::string::size_type p1=orig.find_first_not_of(" \t");
            if (p1==std::string::npos) return "";
            std::string::size_type p2=orig.find_last_not_of(" \t");
            return orig.substr(p1,p2-p1+1);
        }

    public:
        cpuinfo()
        : _M_processor(0)
        {
            std::ifstream fin("/proc/cpuinfo");
            std::string s;
            while ( std::getline(fin,s) ) {
                std::string::size_type p = s.find(':',0);
                if (p == std::string::npos) {
                    _M_processor++;
                    continue;
                }

                std::string tag( trim(s.substr(0,p)) );
                std::string val( trim(s.substr(p+1)) );

                adaptor second(val);
                _M_map[_M_processor].insert( std::make_pair(tag, second) );
            }
        }

        adaptor operator()(int p, const std::string &v)
        {
            if ( p >= CPU_MAX || p >= _M_processor )
                throw std::out_of_range("bad index");

            typename cpu_map::iterator it = _M_map[p].find(adaptor(v));
            if ( it == _M_map[p].end() )
                throw std::runtime_error("bad key");
            return (*it).second;
        }

    };
}

// proc::cpuinfo<> cpu;
// std::cout << static_cast<std::string>(cpu(1,"flags")) << std::endl;

#endif /* CPUINFO_HH */

