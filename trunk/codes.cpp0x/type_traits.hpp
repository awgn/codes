/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TYPE_TRAITS_HPP_
#define _TYPE_TRAITS_HPP_ 

#include <iostream>
#include <type_traits>   
#include <utility>
#include <tuple>         

namespace more 
{
    namespace traits {

    // For use in __is_convertible_simple.
    struct __sfinae_types
    {
      typedef char __one;
      typedef struct { char __arr[2]; } __two;
    };

    // is_class_or_union (using SFINAE... Vandevoorde/Josuttis)
    template <typename T>
    class __is_class_or_union_helper : public __sfinae_types
    {
        template <typename C> static __one test(int C::*);
        template <typename C> static __two test(...);

    public:
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    template <typename T>
    struct is_class_or_union : public std::integral_constant<bool, __is_class_or_union_helper<T>::value>
    {};
    
    // has member type helper (using SFINAE... Vandevoorde/Josuttis)
    #define __has_member_type_helper(abc) \
    template <typename T>   \
    class __has_ ## abc ## _helper : public __sfinae_types   \
    {   \
        template <typename C> static __one test(typename std::remove_reference<typename C::abc>::type *);  \
        template <typename C> static __two test(...);   \
    \
    public: \
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };   \
    }

    __has_member_type_helper(value_type);
    __has_member_type_helper(key_type);
    __has_member_type_helper(mapped_type);
    __has_member_type_helper(container_type);

    __has_member_type_helper(pointer);
    __has_member_type_helper(reference);
    __has_member_type_helper(const_reference);
    __has_member_type_helper(iterator);
    __has_member_type_helper(const_iterator);
    __has_member_type_helper(reverse_iterator);
    __has_member_type_helper(const_reverse_iterator);
    __has_member_type_helper(size_type);
    __has_member_type_helper(difference_type);

    template <typename T>
    struct has_value_type : public std::integral_constant<bool, __has_value_type_helper<T>::value>
    {};

    template <typename t>
    struct has_key_type : public std::integral_constant<bool, __has_key_type_helper<t>::value>
    {};

    template <typename t>
    struct has_mapped_type : public std::integral_constant<bool, __has_mapped_type_helper<t>::value>
    {};

    template <typename t>
    struct has_container_type : public std::integral_constant<bool, __has_container_type_helper<t>::value>
    {};

    template <typename T>
    struct has_pointer : public std::integral_constant<bool, __has_pointer_helper<T>::value>
    {};

    template <typename T>
    struct has_reference : public std::integral_constant<bool, __has_reference_helper<T>::value>
    {};

    template <typename T>
    struct has_const_reference : public std::integral_constant<bool, __has_const_reference_helper<T>::value>
    {};

    template <typename T>
    struct has_iterator : public std::integral_constant<bool, __has_iterator_helper<T>::value>
    {};

    template <typename T>
    struct has_const_iterator : public std::integral_constant<bool, __has_const_iterator_helper<T>::value>
    {};

    template <typename T>
    struct has_reverse_iterator : public std::integral_constant<bool, __has_reverse_iterator_helper<T>::value>
    {};

    template <typename T>
    struct has_const_reverse_iterator : public std::integral_constant<bool, __has_const_reverse_iterator_helper<T>::value>
    {};
    
    template <typename T>
    struct has_size_type : public std::integral_constant<bool, __has_size_type_helper<T>::value>
    {};
    
    template <typename T>
    struct has_difference_type : public std::integral_constant<bool, __has_difference_type_helper<T>::value>
    {};
    

    // is_container 
    
    template <typename T>
    struct is_container : public std::integral_constant<bool, __has_value_type_helper<T>::value && 
                                                              __has_reference_helper<T>::value &&  
                                                              __has_const_reference_helper<T>::value &&  
                                                              __has_iterator_helper<T>::value && 
                                                              __has_const_iterator_helper<T>::value && 
                                                              __has_pointer_helper<T>::value &&  
                                                              __has_size_type_helper<T>::value &&  
                                                              __has_difference_type_helper<T>::value 
                                                               >
    {};

    // is_tuple 

    template <typename T>
    struct is_tuple : public std::integral_constant<bool, false>
    {};

    template <typename ...Ti>
    struct is_tuple<std::tuple<Ti...>> : public std::integral_constant<bool, true>
    {};

    // is_pair

    template <typename T>
    struct is_pair : public std::integral_constant<bool, false>
    {};

    template <typename T, typename U>
    struct is_pair<std::pair<T,U>> : public std::integral_constant<bool, true>
    {};

#if __GNUC__ == 4 &&  __GNUC_MINOR__ > 4

    // has_insertion_operator: operator<<()
    
    template <typename T>
    class has_insertion_operator : public __sfinae_types
    {
        template <typename C> static __one test(typename std::remove_reference<decltype(std::cout << std::declval<C>())>::type *);
        template <typename C> static __two test(...);
    public:    
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

    // has_extraction_operator: operator>>()
    
    template <typename T>
    class has_extraction_operator : public __sfinae_types
    {
        template <typename C> static __one test(typename std::remove_reference<decltype(std::cin >> std::declval<C &>())>::type *);
        template <typename C> static __two test(...);
    public:    
        enum { value = sizeof(test<T>(0)) == sizeof(__one) };
    };

#endif

    } // namespace traits

} // namespace more 

#endif /* _TYPE_TRAITS_HPP_ */
