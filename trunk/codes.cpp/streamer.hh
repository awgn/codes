/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef STREAMER_HH
#define STREAMER_HH

#include <mtp.hh>           // more!
#include <type_traits.hh>   // more!

#include <tr1/type_traits>
#include <tr1/array>
#include <tr1/tuple>

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
                    out << std::tr1::get< std::tr1::tuple_size<T>::value - N>(tupl) << ' ';
                    printon<CharT, Traits, T,N-1>::apply(out,tupl);
                }

            };
            template <typename CharT, typename Traits, typename T>
            struct printon<CharT, Traits, T,0>
            {
                static void apply(std::basic_ostream<CharT, Traits> &, const T &)
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
    inline typename more::mtp::enable_if_c< more::traits::is_container<T>::value && 
    !std::tr1::is_same<typename std::string,T>::value, 
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

#ifndef __GXX_EXPERIMENTAL_CXX0X__
    namespace tr1 
    {
#endif
        ///////////////////////////
        // operator<< for array...

        template <typename CharT, typename Traits, typename T, std::size_t N>
        std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT,Traits> &out, const std::tr1::array<T,N> & rhs)
        {
            out << "[ ";
            more::streamer::tuplarr::printon<CharT, Traits, std::tr1::array<T,N>, N>::apply(out,rhs);
            return out << "]";
        }

        ////////////////////////////////////////////////////////
        // operator<< for tuple: (enabled if T is a tuple<>)... 

        template <typename CharT, typename Traits, typename T>
        typename more::mtp::enable_if< more::traits::is_tuple<T>, std::basic_ostream<CharT,Traits> >::type &
        operator<<(std::basic_ostream<CharT,Traits> &out, const T & rhs)
        {
            out << "< ";
            more::streamer::tuplarr::printon<CharT, Traits, T, std::tr1::tuple_size<T>::value>::apply(out,rhs);
            return out << ">";
        }

#ifndef __GXX_EXPERIMENTAL_CXX0X__
    }   // namespace tr1
#endif

} // namespace std

#endif /* STREAMER_HH */
