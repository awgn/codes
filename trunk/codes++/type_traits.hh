/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef TYPE_TRAITS_HH
#define TYPE_TRAITS_HH

#include <tr1/type_traits>
#include <tr1/tuple>

using std::tr1::integral_constant;
using std::tr1::true_type; using std::tr1::false_type;

namespace more 
{
    // For use in __is_convertible_simple.
    struct __sfinae_types
    {
      typedef char __one;
      typedef struct { char __arr[2]; } __two;
    };

    // is_class (using SFINAE... Vandevoorde/Josuttis)
    template <typename T>
    class __is_class_or_union_helper : public __sfinae_types
    {
        template <typename C> static __one test(int C::*);
        template <typename C> static __two test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    template <typename T>
    struct is_class_or_union : public integral_constant<bool, __is_class_or_union_helper<T>::value>
    {};

    // has member type helper (using SFINAE... Vandevoorde/Josuttis)
    #define __has_member_type_helper(abc) \
    template <typename T>   \
    class __has_ ## abc ## _helper : public __sfinae_types   \
    {   \
        template <typename C> static __one test(typename C::abc *);  \
        template <typename C> static __two test(...);   \
    \
    public: \
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };   \
    }

    __has_member_type_helper(value_type);
    __has_member_type_helper(type);
    __has_member_type_helper(iterator);

    template <typename T>
    struct has_iterator : public integral_constant<bool, __has_iterator_helper<T>::value>
    {};
    template <typename T>
    struct has_value_type : public integral_constant<bool, __has_value_type_helper<T>::value>
    {};
    template <typename T>
    struct has_type : public integral_constant<bool, __has_type_helper<T>::value>
    {};
    template <typename T>
    struct is_container : public integral_constant<bool, __has_iterator_helper<T>::value && 
                                                         __has_value_type_helper<T>::value >
    {};

    // is_tuple 
    template <typename T>
    class __is_tuple_helper : public __sfinae_types
    {
        template <typename C> static __one test(typename std::tr1::tuple_element<0,C>::type *);
        template <typename C> static __two test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    template <typename T>
    struct is_tuple : public integral_constant<bool, __is_tuple_helper<T>::value>
    {};

    // is_metafunction 
    template <typename T>
    class __is_metafunction_helper : public __sfinae_types
    {
        template <typename C> static __one test_value(integral_constant<bool,C::value> *);
        template <typename C> static __two test_value(...);

    public:
        enum { value = __has_type_helper<T>::value || (sizeof(test_value<T>(0)) == sizeof(__one)) };
    };

    template <typename T>
    struct is_metafunction : public integral_constant<bool, __is_metafunction_helper<T>::value>
    {};

}

#endif /* TYPE_TRAITS_HH */
