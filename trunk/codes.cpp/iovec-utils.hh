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
            static typename more::mtp::enable_if< std::is_integral<T>,
                typename std::add_const<T>::type *>::type 
            value(const T &elem)
            {
                return &elem;
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
            static typename more::mtp::enable_if< std::is_integral<T>, size_t>::type 
            value(T)
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
    get_iovec(Iterator __it, Iterator __end, std::forward_iterator_tag)
    {
        std::vector<iovec> ret;

        typedef typename 
            more::mtp::if_< std::tr1::is_same< std::string, typename std::iterator_traits<Iterator>::value_type >::value, 
                char, 
                typename std::iterator_traits<Iterator>::value_type>::type value_type;

        if ( __it == __end )
            return ret;

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

    template <typename Iterator>
    typename more::mtp::enable_if< std::is_integral< typename std::iterator_traits<Iterator>::value_type >,
                std::vector<iovec> >::type
    get_iovec(Iterator __it, Iterator __end, std::random_access_iterator_tag)
    {
        typedef typename std::iterator_traits<Iterator>::value_type value_type;
        typedef typename std::iterator_traits<Iterator>::difference_type difference_type; 

        const difference_type n = std::distance(__it,__end);

        if ( n && n == (&*__end - &*__it) ) 
        {
            iovec iov = { static_cast<void *>(const_cast<value_type *>(& *__it)), n * sizeof(value_type) };
            return std::vector<iovec>(1, iov);
        }

        return get_iovec(__it,__end, std::forward_iterator_tag());
    }

    template <typename Iterator>
    std::vector<iovec>
    get_iovec(Iterator __it, Iterator __end)
    {
        return get_iovec(__it,__end, typename std::iterator_traits<Iterator>::iterator_category());
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
