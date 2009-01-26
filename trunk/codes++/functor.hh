/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef FUNCTOR_HH
#define FUNCTOR_HH

// this (generic) functor implementation bases on automatic-type-erasure
// mechanism [ 9.7.4 C++ Template Metaprogramming: Abrahams/Gurtovoy ]
//

#if   __GNUC__ >= 4
#include <tr1/memory>
#else
#error "g++ compiler not supported"
#endif

namespace generic {

    template <class R, class P>
    class functor
    {
        struct base_functor
        {
            virtual ~base_functor() {}
            virtual R operator()(const P&) const = 0;
        };

        template <class F>
        struct wrapper : base_functor 
        {
            explicit wrapper(F f) 
            : _M_fun(f) 
            {}

            R 
            operator()(const P& x) const
            { return this->_M_fun(x); }

        private:
            F _M_fun;
        };

        std::tr1::shared_ptr<base_functor> _M_ptr;

    public:
        template <class F>
        explicit functor(const F &f) 
        : _M_ptr( new wrapper<F>(f) ) 
        {}

        R 
        operator()(const P &p) const
        { return (*_M_ptr)(p); }

    };

}

#endif /* FUNCTOR_HH */

