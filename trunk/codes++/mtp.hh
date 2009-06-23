/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

// metaprogramming: generic metafunctions 
//

#ifndef MTP_HH
#define MTP_HH

#include <type_traits.hh>
#include <iostream>
#include <string>

namespace mtp {

    class null  {};
    struct empty {};

    template <int n>
    struct tag
    {
        enum { value = n };
    };
    template <typename T>
    struct tag_type
    {
        typedef T type;
    };

    // select type... (ala loki)
    //

    template <bool v, typename U, typename V>
    struct select 
    {
        typedef U type;
    };
    template <typename U, typename V>
    struct select<false, U, V> 
    {
        typedef V type;
    };

    // paramenter optimization ala Loki (Alexandrescu)
    //

    template <typename U> struct __param 
    {
        typedef const U & type;
    };
    template <typename U> struct __param<U &> 
    {
        typedef U& type;
    };

    template <typename T>
    struct param : select< more::traits::is_class_or_union<T>::value, 
                           typename __param<T>::type, 
                           typename tag_type<T>::type >
    {};

    // enable_if_c / disable_if_c ala boost
    // to be used on return type or additional paramenter
    //
    template <bool B, class T = void>
    struct enable_if_c 
    {
        typedef T type;
    };
    template <class T>
    struct enable_if_c<false, T> {};

    template <bool B, class T = void>
    struct disable_if_c 
    {
        typedef T type;
    };
    template <class T>
    struct disable_if_c<true, T> {};

    // enable_if / disable_if ala boost
    // to be used on return type or additional paramenter
    //
    template <typename B, class T = void>
    struct enable_if : public enable_if_c<B::value,T> {}; 

    template <typename B, class T = void>
    struct disable_if : public disable_if_c<B::value,T> {};

    // identity (yalob: yet another layer of abstraction)

    template <typename T>
    struct identity
    {
        typedef T type;
    };

    // if_ and eval_if ala boost

    template <bool Cond, typename TrueType, typename FalseType>
    struct if_
    {
        typedef TrueType type;
    };
    template <typename TrueType, typename FalseType>
    struct if_<false,TrueType, FalseType> 
    {
        typedef FalseType type;
    };

    template <bool Cond, typename TrueType, typename FalseType>
    struct eval_if
    {
        typedef typename TrueType::type type;
    };
    template <typename TrueType, typename FalseType>
    struct eval_if<false,TrueType, FalseType> 
    {
        typedef typename FalseType::type type;
    };

}

#endif /* MTP_HH */
