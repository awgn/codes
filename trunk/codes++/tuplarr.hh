/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TUPLARR_HH_
#define _TUPLARR_HH_ 

#include <tr1/tuple>
#include <tr1/array>
#include <iostream>

#include <mtp.hh>           // mtp::enable_if
#include <type_traits.hh>   // more::traits::is_tuple<>

namespace more 
{
    namespace tuplarr_utils 
    {
        template <typename C, typename T, int N >
        struct pushback_policy 
        {
            static void apply(C &cnt, const T &tupl)
            {
                cnt.push_back( std::tr1::get<std::tr1::tuple_size<T>::value-N>(tupl) );
                pushback_policy<C,T,N-1>::apply(cnt,tupl); 
            }
        };

        template <typename C, typename T>
        struct pushback_policy<C,T,0> 
        {
            static void apply(C &, const T &)
            {}
        };

        template <typename CharT, typename Traits, typename T, int N>
        struct printon_policy
        {
            static void apply(std::basic_ostream<CharT,Traits> &out, const T &tupl)
            {
                out << std::tr1::get< std::tr1::tuple_size<T>::value - N>(tupl) << ' ';
                printon_policy<CharT, Traits, T,N-1>::apply(out,tupl);
            }

        };
        template <typename CharT, typename Traits, typename T>
        struct printon_policy<CharT, Traits, T,0>
        {
            static void apply(std::basic_ostream<CharT, Traits> &out, const T &)
            {}
        };

    }

    template <typename C, typename T>
    static inline 
    void container_backinsert_tuplarr(C &cnt, const T &tupl)
    {
        tuplarr_utils::pushback_policy<C,T,std::tr1::tuple_size<T>::value>::apply(cnt,tupl);
    }

}

namespace std { namespace tr1 {

    ///////////////////////////
    // operator<< for array...

    template <typename CharT, typename Traits, typename T, std::size_t N>
    std::basic_ostream<CharT,Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const std::tr1::array<T,N> & rhs)
    {
        out << "[ ";
        more::tuplarr_utils::printon_policy<CharT, Traits, std::tr1::array<T,N>, N>::apply(out,rhs);
        return out << "]";
    }

    ////////////////////////////////////////////////////////
    // operator<< for tuple: (enabled if T is a tuple<>)... 

    template <typename CharT, typename Traits, typename T>
    typename mtp::enable_if< more::traits::is_tuple<T>, std::basic_ostream<CharT,Traits> >::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T & rhs)
    {
        out << "< ";
        more::tuplarr_utils::printon_policy<CharT, Traits, T, std::tr1::tuple_size<T>::value>::apply(out,rhs);
        return out << ">";
    }

}}

#endif /* _TUPLARR_HH_ */
