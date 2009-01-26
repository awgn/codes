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
using generic::singleton<x>::instance; \
x(const tag &abc,  ## __VA_ARGS__) : generic::singleton<x>(abc)

#define VOLATILE_SINGLETON_CTOR(x, ...)  \
using generic::singleton<x,true>::instance; \
x(const tag &abc,  ## __VA_ARGS__) : generic::singleton<x,true>(abc)

namespace generic
{
    template <typename T, bool VOL = false>
    struct singleton
    {    
        template <bool, typename> struct enable_volatile_if; 
        template <typename U> 
        struct enable_volatile_if<true,U> 
        {
            typedef typename std::tr1::add_volatile<U>::type type;
        };
        template <typename U> 
        struct enable_volatile_if<false,U> 
        {
            typedef U type;
        };

    private:
        singleton();
        singleton(const singleton&);                // noncopyable
        singleton &operator=(const singleton &);    // noncopyable  

    protected:
        ~singleton()
        {}

        struct tag {
            friend class singleton<T, VOL>;
        private:
            tag()  {}
            ~tag() {}
        };

    public:        

        singleton(const singleton::tag &) 
        {}

        // singleton instance...
        //
        static typename enable_volatile_if<VOL,T>::type & instance()
        {
            static typename enable_volatile_if<VOL,T>::type _one_((tag()));
            return _one_;
        }

        // multitons...
        //
        template <typename U>
        static typename enable_volatile_if<VOL,T>::type & instance(const U &u = U())
        {
            static typename enable_volatile_if<VOL,T>::type _n_((tag()), u);
            return _n_;
        }
        template <typename U, typename V>
        static typename enable_volatile_if<VOL,T>::type & instance(const U &u = U(), const V &v = V())
        {
            static typename enable_volatile_if<VOL,T>::type _n_((tag()), u, v);
            return _n_;
        }
        template <typename U, typename V, typename W>
        static typename enable_volatile_if<VOL,T>::type & instance(const U &u = U(), const V &v = V(), const W &w = W())
        {
            static typename enable_volatile_if<VOL,T>::type _n_((tag()), u, v, w);
            return _n_;
        }
        template <typename U, typename V, typename W, typename X>
        static typename enable_volatile_if<VOL,T>::type & instance(const U &u = U(), const V &v = V(), const W &w = W(), 
                                                                   const X &x = X())
        {
            static typename enable_volatile_if<VOL,T>::type _n_((tag()), u, v, w, x);
            return _n_;
        }
        template <typename U, typename V, typename W, typename X, typename Y>
        static typename enable_volatile_if<VOL,T>::type & instance(const U &u = U(), const V &v = V(), const W &w = W(), 
                                                                   const X &x = X(), const Y &y = Y())
        {
            static typename enable_volatile_if<VOL,T>::type _n_((tag()), u, v, w, x, y);
            return _n_;
        }
        template <typename U, typename V, typename W, typename X, typename Y, typename Z>
        static typename enable_volatile_if<VOL,T>::type & instance(const U &u = U(), const V &v = V(), const W &w = W(), 
                                                                   const X &x = X(), const Y &y = Y(), const Z &z = Z())
        {
            static typename enable_volatile_if<VOL,T>::type _n_((tag()), u, v, w, x, y, z);
            return _n_;
        }

    };
}

#endif /* MEYERS_SINGLETON_HH */

