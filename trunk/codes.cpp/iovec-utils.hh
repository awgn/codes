/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _IOVEC_UTILS_HH_
#define _IOVEC_UTILS_HH_ 

#include <sys/uio.h>

#include <tr1/type_traits>
#include <vector>
#include <iterator>
#include <iostream>

#include <mtp.hh>       // more!

namespace more { 

    namespace iovec_util {

        struct address_of {

            template <typename T>
            static typename std::tr1::add_const<T>::type * 
            value(const T &elem)
            {
                return & elem;
            }

            static const char *
            value(const std::string & elem)
            {
                return elem.c_str();
            }

        };

        template <typename Iter>
        struct size_of {

            template <typename T>
            static size_t value(T)
            {
                return sizeof(T);
            }

            static
            size_t value(const std::string &s)
            {
                return s.size();
            }

        };
    }

    template <typename Iterator>
    std::vector<iovec>
    get_iovec(Iterator __it, Iterator __end)
    {
        typedef typename 
            more::mtp::if_< std::tr1::is_same< std::string, typename std::iterator_traits<Iterator>::value_type >::value, 
                char, 
                typename std::iterator_traits<Iterator>::value_type>::type value_type;

        std::vector<iovec> ret;

        if ( __it == __end )
            return std::vector<iovec>();

        const value_type * base = iovec_util::address_of::value(*__it); size_t len = 0;

        for(const value_type * ptr = base; __it != __end;  len += iovec_util::size_of<Iterator>::value(*__it++), ++ptr )
        {
            if ( ptr != iovec_util::address_of::value(*__it))
            {
                iovec iov = { static_cast<void *>(const_cast<value_type *>(base)), len };
                ret.push_back(iov);
                base = ptr = iovec_util::address_of::value(*__it);
                len  = 0;
            }
        }
       
        iovec iov = { static_cast<void *>(const_cast<value_type *>(base)), len };
        ret.push_back(iov);
        return ret;
    }    

} // nnamespace more

namespace std {

    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, const iovec &iov)
    {
        out << '{' << static_cast<void *>(iov.iov_base) << ':' << iov.iov_len << '}';
        return out;
    }
}

#endif /* _IOVEC_UTILS_HH_ */
