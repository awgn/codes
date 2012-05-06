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

// this generic functor implementation bases on automatic-type-erasure
// mechanism [ 9.7.4 C++ Template Metaprogramming: Abrahams/Gurtovoy ]
//

#include <tr1/memory>    

namespace more {

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
            : m_fun(f) 
            {}

            R 
            operator()(const P& x) const
            { return this->m_fun(x); }

        private:
            F m_fun;
        };

        std::tr1::shared_ptr<base_functor> m_ptr;

    public:
        template <class F>
        explicit functor(const F &f) 
        : m_ptr( new wrapper<F>(f) ) 
        {}

        R 
        operator()(const P &p) const
        { return (*m_ptr)(p); }

    };

}

#endif /* FUNCTOR_HH */

