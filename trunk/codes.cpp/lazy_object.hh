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

#include <any.hh>           // more!

#include <tr1/memory>    
#include <stdexcept>

namespace more {

    template <typename T> 
    class lazy_object
    {
    public:
        lazy_object()
        : m_obj()
        {}

        template <typename U>
        lazy_object(const lazy_object<U> &rhs)        
        : m_obj (rhs.obj()),
          m_arg1(rhs.arg1()),
          m_arg2(rhs.arg2()),
          m_arg3(rhs.arg3()),
          m_arg4(rhs.arg4()),
          m_arg5(rhs.arg5()),
          m_arg6(rhs.arg6())
        {}

        // other constructors...

        template <typename T1>
        lazy_object(const T1 &t1)
        : m_obj(), m_arg1(t1)
        {}

        template <typename T1, typename T2>
        lazy_object(const T1 &t1, const T2 &t2)
        : m_obj(), m_arg1(t1), m_arg2(t2)
        {}

        template <typename T1, typename T2, typename T3>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3)
        : m_obj(), m_arg1(t1), m_arg2(t2), m_arg3(t3)
        {}

        template <typename T1, typename T2, typename T3, typename T4>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
        : m_obj(), m_arg1(t1), m_arg2(t2), m_arg3(t3), m_arg4(t4)
        {}

        template <typename T1, typename T2, typename T3, typename T4, typename T5>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
        : m_obj(), m_arg1(t1), m_arg2(t2), m_arg3(t3), m_arg4(t4), m_arg5(t5)
        {}

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
        : m_obj(), m_arg1(t1), m_arg2(t2), m_arg3(t3), m_arg4(t4), m_arg5(t5), m_arg6(t6)
        {}

        ~lazy_object()
        {}

        ///////////////// delayed constructor

        void ctor()
        {
            m_obj = std::tr1::shared_ptr<T>( new T );
        }

        template <typename P1>
        void ctor() 
        {
            m_obj = typename std::tr1::shared_ptr<T>( new T(more::any_cast<P1>(m_arg1)) );
        }
        template <typename P1, typename P2>
        void ctor() 
        {
            m_obj = typename std::tr1::shared_ptr<T>( new T(more::any_cast<P1>(m_arg1),
                                                               more::any_cast<P2>(m_arg2)) );
        }
        template <typename P1, typename P2, typename P3>
        void ctor() 
        {
            m_obj = typename std::tr1::shared_ptr<T>( new T(more::any_cast<P1>(m_arg1),
                                                               more::any_cast<P2>(m_arg2),
                                                               more::any_cast<P3>(m_arg3)) );
        }
        template <typename P1, typename P2, typename P3, typename P4>
        void ctor()
        {
            m_obj = typename std::tr1::shared_ptr<T>( new T(more::any_cast<P1>(m_arg1),
                                                               more::any_cast<P2>(m_arg2),
                                                               more::any_cast<P3>(m_arg3),
                                                               more::any_cast<P4>(m_arg4)) );
        }
        template <typename P1, typename P2, typename P3, typename P4, typename P5>
        void ctor()
        {
            m_obj = typename std::tr1::shared_ptr<T>( new T(more::any_cast<P1>(m_arg1),
                                                               more::any_cast<P2>(m_arg2),
                                                               more::any_cast<P3>(m_arg3),
                                                               more::any_cast<P4>(m_arg4),
                                                               more::any_cast<P5>(m_arg5)) );
        }
        template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
        void ctor()
        {
            m_obj = typename std::tr1::shared_ptr<T>( new T(more::any_cast<P1>(m_arg1),
                                                               more::any_cast<P2>(m_arg2),
                                                               more::any_cast<P3>(m_arg3),
                                                               more::any_cast<P4>(m_arg4),
                                                               more::any_cast<P5>(m_arg5),
                                                               more::any_cast<P6>(m_arg6)) );
        }

        typename std::tr1::shared_ptr<T>
        shared_from_this() const 
        {
            if (!m_obj)
                throw std::logic_error("object not yet constructed");

            return typename std::tr1::shared_ptr<T>(m_obj);
        }

        // enable upcast...
        template <typename U> 
        operator lazy_object<U>()
        {
            return lazy_object<U>(*this);
        }

        // internals readers...
        const typename std::tr1::shared_ptr<T> &
        obj() const
        {
            return m_obj;
        }
       
        const more::any &
        arg1() const
        {
           return m_arg1;
        } 
        const more::any &
        arg2() const
        {
           return m_arg2;
        } 
        const more::any &
        arg3() const
        {
           return m_arg3;
        } 
        const more::any &
        arg4() const
        {
           return m_arg4;
        } 
        const more::any &
        arg5() const
        {
           return m_arg5;
        } 
        const more::any &
        arg6() const
        {
           return m_arg6;
        } 

    private:
        typename std::tr1::shared_ptr<T> m_obj; // read why more::shared_prt<> in shared_ptr.hh 

        more::any  m_arg1;  
        more::any  m_arg2; 
        more::any  m_arg3;
        more::any  m_arg4; 
        more::any  m_arg5; 
        more::any  m_arg6;
    };
}

#endif /* _LAZY_OBJECT_HH_ */
