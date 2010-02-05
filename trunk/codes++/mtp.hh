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

#include <iostream>
#include <string>

namespace more { namespace mtp {

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

    //////////////////////////////////////////////////////
    // Loki's heritage
    //////////////////////////////////////////////////////

    template <int n>
    struct int2type
    {
        enum { value = n };
    };

    template <typename T>
    struct type2type
    {
        typedef T type;
    }; 

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

    // parameter optimization: (deprecated)
    // template <typename T>
    // struct parameter : select< more::traits::is_class_or_union<T>::value, 
    //                            typename std::tr1::add_reference<T>::type, 
    //                            typename tag_type<T>::type >
    // {};

    //////////////////////////////////////////////////////
    // boost's heritage
    //////////////////////////////////////////////////////

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

    // if_ and eval_if (same as Loki select) 
    //

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

} // namepsace mtp
} // namespace more

#endif /* MTP_HH */
