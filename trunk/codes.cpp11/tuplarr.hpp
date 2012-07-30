/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TUPLARR_HPP_
#define _TUPLARR_HPP_ 

#include <type_traits.hpp>   // more::traits::is_tuple<>
#include <streamer.hpp>      // more!

#include <iostream>
#include <iterator>
#include <algorithm>
#include <tuple>      
#include <array>      
#include <functional> 
 
namespace more 
{
    namespace tuplarr_policy 
    {
        // assign policy 
        //
       
        template <typename Iter, typename T, int N>
        struct assign 
        {
            static void apply(const T &tupl, Iter &it)
            {
                *it++ = std::get<std::tuple_size<T>::value-N>(tupl);
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
                f(std::get<std::tuple_size<T>::value-N>(tupl));
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
                return (elem == std::get<std::tuple_size<T>::value-N>(tupl)) + count<T,E,N-1>::apply(tupl,elem);
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
                return f(std::get<std::tuple_size<T>::value-N>(tupl)) + count_if<T,Fun,N-1>::apply(tupl,f);
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
                out << std::get< std::tuple_size<T>::value - N>(tupl) << ' ';
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

    ////////////////////////////// algorithms //////////////////////////////

    namespace tuplarr {

        // copy algorithm
        //

        template <typename T, typename Iter>
        inline
        void copy(const T &tupl, Iter out)
        {
            tuplarr_policy::assign<Iter,T,std::tuple_size<T>::value>::apply(tupl,out);
        } 

        // for_each algorithm 
        //

        template <typename T, typename Fun>
        inline
        void for_each(const T &tupl, Fun f)
        {
            tuplarr_policy::foreach<T, Fun, std::tuple_size<T>::value>::apply(tupl,f);
        } 

        template <typename T, typename E>
        inline
        int count(const T &tupl, E elem)
        {
            return tuplarr_policy::count<T, E, std::tuple_size<T>::value>::apply(tupl,elem);
        } 

        template <typename T, typename Fun>
        inline
        int count_if(const T &tupl, Fun f)
        {
            return tuplarr_policy::count_if<T, Fun, std::tuple_size<T>::value>::apply(tupl,f);
        } 

        // To be completed...
    }
}

#endif /* _TUPLARR_HPP_ */
