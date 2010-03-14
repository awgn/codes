/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _CPUINFO_HH_
#define _CPUINFO_HH_ 

#include <iomanip.hh>       // more
#include <string-utils.hh>  // more

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

namespace more { namespace proc { 

    class cpuinfo
    { 
    private:
        typedef std::map<std::string, std::string> map_type;
        std::vector<map_type> _M_maps;

    public:
        cpuinfo(const char *p = "/proc/cpuinfo")
        : _M_maps()
        {
            std::ifstream pin(p);
            if (!p)
                throw std::runtime_error(p);     

            more::string_token key(":");
            more::string_token value("\n");

            std::map<std::string,std::string> m;

            while(pin >> key)
            {
                pin >> value;
                more::trim(static_cast<std::string &>(key));
                more::trim(static_cast<std::string &>(value));

                if ( !static_cast<std::string &>(key).compare("processor")) {
                    if (m.size())
                        _M_maps.push_back(m);
                    m.clear();
                }
                m.insert(std::make_pair(key,value));
            }

            _M_maps.push_back(m);
        }

        std::string
        operator()(std::vector<map_type>::size_type p, const std::string &k) const
        {
            if ( p >= _M_maps.size() )
                throw std::out_of_range("bad index");
            map_type::const_iterator it = _M_maps[p].find(k);
            if ( it == _M_maps[p].end() )
                throw std::runtime_error("bad key");
            return (*it).second;
        }

        int
        size() const
        { return _M_maps.size(); }

    };

} // namespace proc
} // namespace more

#endif /* _CPUINFO_HH_ */
