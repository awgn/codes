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
#include <tr1_memory.hh>    // more!

#include <stdexcept>

namespace more {

    template <typename T> 
    class lazy_object
    {
    public:
        lazy_object()
        : _M_obj()
        {}

        template <typename U>
        lazy_object(const lazy_object<U> &rhs)        
        : _M_obj (rhs.obj()),
          _M_arg1(rhs.arg1()),
          _M_arg2(rhs.arg2()),
          _M_arg3(rhs.arg3()),
          _M_arg4(rhs.arg4()),
          _M_arg5(rhs.arg5()),
          _M_arg6(rhs.arg6())
        {}

        // other constructors...

        template <typename T1>
        lazy_object(const T1 &t1)
        : _M_obj(), _M_arg1(t1)
        {}

        template <typename T1, typename T2>
        lazy_object(const T1 &t1, const T2 &t2)
        : _M_obj(), _M_arg1(t1), _M_arg2(t2)
        {}

        template <typename T1, typename T2, typename T3>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3)
        : _M_obj(), _M_arg1(t1), _M_arg2(t2), _M_arg3(t3)
        {}

        template <typename T1, typename T2, typename T3, typename T4>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
        : _M_obj(), _M_arg1(t1), _M_arg2(t2), _M_arg3(t3), _M_arg4(t4)
        {}

        template <typename T1, typename T2, typename T3, typename T4, typename T5>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
        : _M_obj(), _M_arg1(t1), _M_arg2(t2), _M_arg3(t3), _M_arg4(t4), _M_arg5(t5)
        {}

        template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
        lazy_object(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5, const T6 &t6)
        : _M_obj(), _M_arg1(t1), _M_arg2(t2), _M_arg3(t3), _M_arg4(t4), _M_arg5(t5), _M_arg6(t6)
        {}

        ~lazy_object()
        {}

        ///////////////// delayed constructor

        void ctor()
        {
            _M_obj = std::shared_ptr<T>( new T );
        }

        template <typename P1>
        void ctor() 
        {
            _M_obj = typename std::shared_ptr<T>( new T(more::any_cast<P1>(_M_arg1)) );
        }
        template <typename P1, typename P2>
        void ctor() 
        {
            _M_obj = typename std::shared_ptr<T>( new T(more::any_cast<P1>(_M_arg1),
                                                               more::any_cast<P2>(_M_arg2)) );
        }
        template <typename P1, typename P2, typename P3>
        void ctor() 
        {
            _M_obj = typename std::shared_ptr<T>( new T(more::any_cast<P1>(_M_arg1),
                                                               more::any_cast<P2>(_M_arg2),
                                                               more::any_cast<P3>(_M_arg3)) );
        }
        template <typename P1, typename P2, typename P3, typename P4>
        void ctor()
        {
            _M_obj = typename std::shared_ptr<T>( new T(more::any_cast<P1>(_M_arg1),
                                                               more::any_cast<P2>(_M_arg2),
                                                               more::any_cast<P3>(_M_arg3),
                                                               more::any_cast<P4>(_M_arg4)) );
        }
        template <typename P1, typename P2, typename P3, typename P4, typename P5>
        void ctor()
        {
            _M_obj = typename std::shared_ptr<T>( new T(more::any_cast<P1>(_M_arg1),
                                                               more::any_cast<P2>(_M_arg2),
                                                               more::any_cast<P3>(_M_arg3),
                                                               more::any_cast<P4>(_M_arg4),
                                                               more::any_cast<P5>(_M_arg5)) );
        }
        template <typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
        void ctor()
        {
            _M_obj = typename std::shared_ptr<T>( new T(more::any_cast<P1>(_M_arg1),
                                                               more::any_cast<P2>(_M_arg2),
                                                               more::any_cast<P3>(_M_arg3),
                                                               more::any_cast<P4>(_M_arg4),
                                                               more::any_cast<P5>(_M_arg5),
                                                               more::any_cast<P6>(_M_arg6)) );
        }

        typename std::shared_ptr<T>
        shared_from_this() const 
        {
            if (!_M_obj)
                throw std::logic_error("object not yet constructed");

            return typename std::shared_ptr<T>(_M_obj);
        }

        // enable upcast...
        template <typename U> 
        operator lazy_object<U>()
        {
            return lazy_object<U>(*this);
        }

        // internals readers...
        const typename std::shared_ptr<T> &
        obj() const
        {
            return _M_obj;
        }
       
        const more::any &
        arg1() const
        {
           return _M_arg1;
        } 
        const more::any &
        arg2() const
        {
           return _M_arg2;
        } 
        const more::any &
        arg3() const
        {
           return _M_arg3;
        } 
        const more::any &
        arg4() const
        {
           return _M_arg4;
        } 
        const more::any &
        arg5() const
        {
           return _M_arg5;
        } 
        const more::any &
        arg6() const
        {
           return _M_arg6;
        } 

    private:
        typename std::shared_ptr<T> _M_obj; // read why more::shared_prt<> in shared_ptr.hh 

        more::any  _M_arg1;  
        more::any  _M_arg2; 
        more::any  _M_arg3;
        more::any  _M_arg4; 
        more::any  _M_arg5; 
        more::any  _M_arg6;
    };
}

#endif /* _LAZY_OBJECT_HH_ */
