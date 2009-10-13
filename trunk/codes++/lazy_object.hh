/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _LAZY_OBJECT_HH_
#define _LAZY_OBJECT_HH_ 

#include <template_util.hh>
#include <shared_ptr.hh>

namespace more {

    template <typename T, typename T1 = null_type, typename T2 = null_type, typename T3 = null_type, 
                          typename T4 = null_type, typename T5 = null_type, typename T6 = null_type >
    struct object_allocator
    {
        static
        T * alloc(const T1 & t1, const T2 & t2, const T3 & t3, 
                  const T4 & t4, const T5 & t5, const T6 & t6)
        {
           return alloc(more::tag< count_param_6<T1,T2,T3,T4,T5,T6>::value >(), t1, t2, t3, t4, t5, t6); 
        } 

        static
        T * alloc( more::tag<0>, const T1 &, const T2 &, const T3 &,const T4 &, const T5 &, const T6 &)
        {
            return new T;
        }
        static
        T * alloc( more::tag<1>, const T1 &t1, const T2 &, const T3 &, const T4 &, const T5 &, const T6 &)
        {
            return new T(t1);
        }
        static
        T * alloc( more::tag<2>, const T1 &t1, const T2 &t2, const T3 &, const T4 &, const T5 &, const T6 &)
        {
            return new T(t1,t2);
        }
        static
        T * alloc( more::tag<3>, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &, const T5 &, const T6 &)
        {
            return new T(t1,t2,t3);
        }
        static
        T * alloc( more::tag<4>, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &, const T6 &)
        {
            return new T(t1,t2,t3,t4);
        }
        static
        T * alloc( more::tag<5>, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &)
        {
            return new T(t1,t2,t3,t4,t5);
        }
        static
        T * alloc( more::tag<6>, const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
        {
            return new T(t1,t2,t3,t4,t5,t6);
        }

    };

    template <typename T, typename T1 = null_type, typename T2 = null_type, typename T3 = null_type, 
                          typename T4 = null_type, typename T5 = null_type, typename T6 = null_type >
    struct lazy_object
    {
        lazy_object(const T1 & t1 = T1(), const T2 & t2 = T2(), const T3 & t3 = T3(), 
                    const T4 & t4 = T4(), const T5 & t5 = T5(), const T6 & t6 = T6())

        : _M_obj(), _M_t1(t1), _M_t2(t2), _M_t3(t3), _M_t4(t4), _M_t5(t5), _M_t6(t6)
        {}

        ~lazy_object()
        {}

        ///////////////// delayed constructor

        void operator()()
        {
            _M_obj = typename more::shared_ptr<T>::type(object_allocator<T,T1,T2,T3,T4,T5,T6>::alloc(_M_t1, _M_t2, _M_t3, _M_t4, _M_t5, _M_t6) );
        }

        typename more::shared_ptr<T>::type
        shared_from_this()
        {
            return typename more::shared_ptr<T>::type(_M_obj);
        }

        typename more::shared_ptr<T>::type _M_obj; // read why more::shared_prt<> in shared_ptr.hh 

        T1  _M_t1; T2  _M_t2; T3  _M_t3;
        T4  _M_t4; T5  _M_t5; T6  _M_t6;
    };
}

#endif /* _LAZY_OBJECT_HH_ */
