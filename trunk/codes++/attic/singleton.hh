/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef MEYERS_SINGLETON_HH
#define MEYERS_SINGLETON_HH

#include <tr1/type_traits>

//
// My singleton implementation inspired to that of Meyers. It is thread safe 
// when compiled with g++ 4.0 or higher 
//

#define SINGLETON_CTOR(x, ...)  \
using more::singleton<x>::instance; \
x(const tag &abc,  ## __VA_ARGS__) : more::singleton<x>(abc)

#define CONST_SINGLETON_CTOR(x, ...)  \
using more::singleton<x, const more::singleton_type>::instance; \
x(const tag &abc,  ## __VA_ARGS__) : more::singleton<x, const more::singleton_type>(abc)

#define VOLATILE_SINGLETON_CTOR(x, ...)  \
using more::singleton<x, volatile more::singleton_type>::instance; \
x(const tag &abc,  ## __VA_ARGS__) : more::singleton<x, volatile more::singleton_type>(abc)

#define CONST_VOLATILE_SINGLETON_CTOR(x, ...)  \
using more::singleton<x, const volatile more::singleton_type>::instance; \
x(const tag &abc,  ## __VA_ARGS__) : more::singleton<x, const volatile more::singleton_type>(abc)

#define TEMPLATE_SINGLETON_CTOR(x, ...)  \
using more::singleton<x>::instance; \
x(const typename more::singleton<x>::tag &abc,  ## __VA_ARGS__) : more::singleton<x>(abc)

#define CONST_TEMPLATE_SINGLETON_CTOR(x, ...)  \
using more::singleton<x, const more::singleton_type>::instance; \
x(const typename more::singleton<x, const more::singleton_type>::tag &abc,  ## __VA_ARGS__) : more::singleton<x, const more::singleton_type>(abc)

#define VOLATILE_TEMPLATE_SINGLETON_CTOR(x, ...)  \
using more::singleton<x, volatile more::singleton_type>::instance; \
x(const typename more::singleton<x, volatile more::singleton_type>::tag &abc,  ## __VA_ARGS__) : more::singleton<x, volatile more::singleton_type>(abc)

#define CONST_VOLATILE_TEMPLATE_SINGLETON_CTOR(x, ...)  \
using more::singleton<x, const volatile more::singleton_type>::instance; \
x(const typename more::singleton<x, const volatile more::singleton_type>::tag &abc,  ## __VA_ARGS__) : more::singleton<x, const volatile more::singleton_type>(abc)


namespace more 
{

    struct singleton_type {};

    template <typename T, typename CV = singleton_type >
    struct singleton
    {   
        template <typename U, bool c, bool v> struct __add_cv;

        template <typename U>
        struct __add_cv<U, true, false>
        {
            typedef typename std::tr1::add_const<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, false, true>
        {
            typedef typename std::tr1::add_volatile<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, true, true>
        {
            typedef typename std::tr1::add_cv<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, false, false>
        {
            typedef U type;
        };
       
        template <typename U, typename V>
        struct add_cv_qualifier
        {
            typedef typename __add_cv<U, std::tr1::is_const<V>::value, std::tr1::is_volatile<V>::value>::type type;
        }; 

    private:
        singleton();
        singleton(const singleton&);                // noncopyable
        singleton &operator=(const singleton &);    // noncopyable  

    protected:
        ~singleton()
        {}

        struct tag {
            friend class singleton<T, CV>;
        private:
            tag()  {}
            ~tag() {}
        };

    public:        

        singleton(const singleton::tag &) 
        {}

        // singleton instance...
        //
        static typename add_cv_qualifier<T,CV>::type & instance()
       {
            static typename add_cv_qualifier<T,CV>::type _one_((tag()));
            return _one_;
        }

        // multitons...
        //
        template <typename U>
        static typename add_cv_qualifier<T,CV>::type & instance(const U &u = U())
        {
            static typename add_cv_qualifier<T,CV>::type _n_((tag()), u);
            return _n_;
        }
        template <typename U, typename V>
        static typename add_cv_qualifier<T,CV>::type & instance(const U &u = U(), const V &v = V())
        {
            static typename add_cv_qualifier<T,CV>::type  _n_((tag()), u, v);
            return _n_;
        }
        template <typename U, typename V, typename W>
        static typename add_cv_qualifier<T,CV>::type & instance(const U &u = U(), const V &v = V(), const W &w = W())
        {
            static typename add_cv_qualifier<T,CV>::type _n_((tag()), u, v, w);
            return _n_;
        }
        template <typename U, typename V, typename W, typename X>
        static typename add_cv_qualifier<T,CV>::type & instance(const U &u = U(), const V &v = V(), const W &w = W(), 
                                                                   const X &x = X())
        {
            static typename add_cv_qualifier<T,CV>::type _n_((tag()), u, v, w, x);
            return _n_;
        }
        template <typename U, typename V, typename W, typename X, typename Y>
        static typename add_cv_qualifier<T,CV>::type & instance(const U &u = U(), const V &v = V(), const W &w = W(), 
                                                                   const X &x = X(), const Y &y = Y())
        {
            static typename add_cv_qualifier<T,CV>::type _n_((tag()), u, v, w, x, y);
            return _n_;
        }
        template <typename U, typename V, typename W, typename X, typename Y, typename Z>
        static typename add_cv_qualifier<T,CV>::type & instance(const U &u = U(), const V &v = V(), const W &w = W(), 
                                                                   const X &x = X(), const Y &y = Y(), const Z &z = Z())
        {
            static typename add_cv_qualifier<T,CV>::type _n_((tag()), u, v, w, x, y, z);
            return _n_;
        }

    };
}

#endif /* MEYERS_SINGLETON_HH */

