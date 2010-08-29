/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_ 

#include <iostream>
#include <type_traits>

namespace more
{    
    struct singleton_type {};
    struct indestructible_singleton_type {};

    // base_class:
                
    template <typename T, typename Ty, typename U = typename std::remove_cv<Ty>::type /* for partial specialization */ > class singleton;

    struct singleton_base
    {
        template <typename T>
        struct identity
        {
            typedef T value_type;
        };

        template <typename U, bool c, bool v> struct __add_cv;
        template <typename U>
        struct __add_cv<U, true, false>
        {
            typedef typename std::add_const<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, false, true>
        {
            typedef typename std::add_volatile<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, true, true>
        {
            typedef typename std::add_cv<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, false, false>
        {
            typedef U type;
        };

        template <typename U, typename V>
        struct add_cv_qualifier
        {
            typedef typename __add_cv<U, std::is_const<V>::value, std::is_volatile<V>::value>::type type;
        };

    private:
        singleton_base(const singleton_base&) = delete;                // noncopyable
        singleton_base &operator=(const singleton_base &) = delete;    // noncopyable  

    protected:
        singleton_base() = default;
        ~singleton_base() = default;
    };

    
    ///////////////////////////////////////////////// singleton_type 


    template <typename T, typename Ty> 
    class singleton<T, Ty, singleton_type>  : public singleton_base
    {
        friend class singleton_base::identity<T>::value_type;
        struct tag {};

    public:
        typedef singleton<T,Ty,singleton_type> base_type;
        
        // instance...
        template <typename ... Arg>
        static typename singleton_base::add_cv_qualifier<T,Ty>::type& 
        instance(Arg&& ... arg)
        {
            static typename singleton_base::add_cv_qualifier<T,Ty>::type one(typename base_type::tag(), arg...);
            return one;
        }
    };

    ///////////////////////////////////////////////// indestructible_singleton_type 

    template <typename T, typename Ty>
    class singleton<T, Ty, indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::value_type;
        struct tag {};

    public:
        typedef singleton<T,Ty,indestructible_singleton_type> base_type;

        // instance...
        template <typename ... Arg>
        static typename singleton_base::add_cv_qualifier<T,Ty>::type& 
        instance(Arg && ...arg)
        {
            static typename singleton_base::add_cv_qualifier<T,Ty>::type * one = 
                new typename singleton_base::add_cv_qualifier<T,Ty>::type(typename base_type::tag(), arg...);
            return *one;
        }
    };

} // namespace more

#endif /* _SINGLETON_HPP_ */
