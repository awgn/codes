/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _STREAMER_HPP_
#define _STREAMER_HPP_

#include <type_traits.hpp>  // more!
#include <type_traits>
#include <array>
#include <tuple>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <cstring>

namespace more {

    namespace streamer {

       // construction-on-the-first idiom ensures sep_index will be unique  
        //

        static int sep_index()
        {
            static int index = std::ios_base::xalloc();
            return index;
        }    

        template <typename CharT, typename Traits>
        std::basic_ostream<CharT, Traits> &
        sep(std::basic_ostream<CharT, Traits> &out, const char * sep = NULL)
        {
            free(reinterpret_cast<void *>(out.iword(sep_index())));
            out.iword(sep_index()) = reinterpret_cast<long>(sep ? strdup(sep) : 0);
            return out;
        }    

        namespace tuplarr {

            // printon policy 
            //

            template <typename CharT, typename Traits, typename T, int N>
            struct printon
            {
                static void apply(std::basic_ostream<CharT,Traits> &out, const T &tupl)
                {
                    out << std::get< std::tuple_size<T>::value - N>(tupl) << ' ';
                    printon<CharT, Traits, T,N-1>::apply(out,tupl);
                }

            };
            template <typename CharT, typename Traits, typename T>
            struct printon<CharT, Traits, T,0>
            {
                static void apply(std::basic_ostream<CharT, Traits> &out, const T &)
                {}
            };
        }
    }
}

namespace std {

    ///////////////////////////////////////
    // operator<< for generic containers...
    //

    template <typename CharT, typename Traits, typename T>
    inline typename std::enable_if< more::traits::is_container<T>::value && 
    !is_same<typename std::string,T>::value, 
    std::basic_ostream<CharT,Traits> >::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T &v)
    {
        std::copy(v.begin(), v.end(), 
                  std::ostream_iterator<typename T::value_type>(out, reinterpret_cast<char *>(out.iword(more::streamer::sep_index()))));
        return out;
    };

    //////////////////////////
    // operator<< for pair...

    template <typename CharT, typename Traits, typename U, typename V>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, const std::pair<U,V> &r)
    {
        return out << '<' << r.first << ':' << r.second << '>';
    }

    ///////////////////////////
    // operator<< for array...

    template <typename CharT, typename Traits, typename T, std::size_t N>
    std::basic_ostream<CharT,Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const std::array<T,N> &rhs)
    {
        out << "[ ";
        more::streamer::tuplarr::printon<CharT, Traits, std::array<T,N>, N>::apply(out,rhs);
        return out << "]";
    }

    ////////////////////////////////////////////////////////
    // operator<< for tuple: (enabled if T is a tuple<>)... 

    template <typename CharT, typename Traits, typename T>
    typename std::enable_if< more::traits::is_tuple<T>::value, std::basic_ostream<CharT,Traits> >::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T &rhs)
    {
        out << "< ";
        more::streamer::tuplarr::printon<CharT, Traits, T, std::tuple_size<T>::value>::apply(out,rhs);
        return out << ">";
    }

} // namespace std

#endif /* _STREAMER_HPP_ */
