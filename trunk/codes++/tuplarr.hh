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
#include <iterator>

#include <mtp.hh>           // mtp::enable_if
#include <type_traits.hh>   // more::traits::is_tuple<>

namespace more 
{
    namespace tuplarr_policy 
    {
        // assign policy 
        //
       
        template <typename Iter, typename T, int N >
        struct assign 
        {
            static void apply(const T &tupl, Iter &it)
            {
                *it++ = std::tr1::get<std::tr1::tuple_size<T>::value-N>(tupl);
                assign<Iter,T,N-1>::apply(tupl,it); 
            }
        };

        template <typename Iter, typename T>
        struct assign<Iter,T,0> 
        {
            static void apply(const T &, Iter &)
            {}
        };

        // foreach policy 
        //

        template <typename T, typename Fun, int N >
        struct foreach 
        {
            static void apply(const T &tupl, Fun f)
            {
                f(std::tr1::get<std::tr1::tuple_size<T>::value-N>(tupl));
                foreach<T,Fun,N-1>::apply(tupl,f); 
            }
        };

        template <typename T, typename Fun>
        struct foreach<T,Fun,0> 
        {
            static void apply(const T &, Fun)
            {}
        };

        // count policy
        //

        template <typename T, typename E, int N >
        struct count 
        {
            static int apply(const T &tupl, E elem)
            {
                return (elem == std::tr1::get<std::tr1::tuple_size<T>::value-N>(tupl)) + count<T,E,N-1>::apply(tupl,elem);
            }
        };

        template <typename T, typename E>
        struct count<T,E,0> 
        {
            static int apply(const T &, E)
            {
                return 0;
            }
        };

        // count_if policy
        //

        template <typename T, typename Fun, int N >
        struct count_if 
        {
            static int apply(const T &tupl, Fun f)
            {
                return f(std::tr1::get<std::tr1::tuple_size<T>::value-N>(tupl)) + count_if<T,Fun,N-1>::apply(tupl,f);
            }
        };

        template <typename T, typename Fun>
        struct count_if<T,Fun,0> 
        {
            static int apply(const T &, Fun)
            {
                return 0;
            }
        };

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
            static void apply(std::basic_ostream<CharT, Traits> &out, const T &)
            {}
        };

    }

    ////////////////////////////// algorithms //////////////////////////////

    namespace tuplarr {

        // copy algorith
        //

        template <typename T, typename Iter>
        inline
        void copy(const T &tupl, Iter out)
        {
            tuplarr_policy::assign<Iter,T,std::tr1::tuple_size<T>::value>::apply(tupl,out);
        } 

        // for_earch algorithm 
        //

        template <typename T, typename Fun>
        inline
        void for_each(const T &tupl, Fun f)
        {
            tuplarr_policy::foreach<T, Fun, std::tr1::tuple_size<T>::value>::apply(tupl,f);
        } 

        template <typename T, typename E>
        inline
        int count(const T &tupl, E elem)
        {
            return tuplarr_policy::count<T, E, std::tr1::tuple_size<T>::value>::apply(tupl,elem);
        } 

        template <typename T, typename Fun>
        inline
        int count_if(const T &tupl, Fun f)
        {
            return tuplarr_policy::count_if<T, Fun, std::tr1::tuple_size<T>::value>::apply(tupl,f);
        } 

        // ...to complete bacause it's a very tedious implementation -- yawn!!!
        // Nicola...

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
        more::tuplarr_policy::printon<CharT, Traits, std::tr1::array<T,N>, N>::apply(out,rhs);
        return out << "]";
    }

    ////////////////////////////////////////////////////////
    // operator<< for tuple: (enabled if T is a tuple<>)... 

    template <typename CharT, typename Traits, typename T>
    typename mtp::enable_if< more::traits::is_tuple<T>, std::basic_ostream<CharT,Traits> >::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T & rhs)
    {
        out << "< ";
        more::tuplarr_policy::printon<CharT, Traits, T, std::tr1::tuple_size<T>::value>::apply(out,rhs);
        return out << ">";
    }

}}

#endif /* _TUPLARR_HH_ */
