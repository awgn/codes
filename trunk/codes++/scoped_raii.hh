/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SCOPED_RAII_HH_
#define _SCOPED_RAII_HH_ 

#include <noncopyable.hh>

namespace more { 

    template <typename T, void (T::*ctor)(), void (T::*dtor)() >
    class scoped_raii : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii<T, ctor, dtor> base_type;

        explicit scoped_raii(device_type &dev)
        : _M_device(dev)
        {
            (_M_device.*ctor)();
        }

        ~scoped_raii()
        {
            (_M_device.*dtor)();
        }

    private:
        device_type & _M_device;
    };

    template <typename T, typename P1, void (T::*ctor)(P1), void (T::*dtor)() >
    class scoped_raii_1 : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii_1<T, P1, ctor, dtor> base_type;

        explicit scoped_raii_1(device_type &dev,const P1 & p1)
        : _M_device(dev)
        {
            (_M_device.*ctor)(p1);
        }

        ~scoped_raii_1()
        {
            (_M_device.*dtor)();
        }

    private:
        device_type & _M_device;
    };

    template <typename T, typename P1, typename P2, void (T::*ctor)(P1,P2), void (T::*dtor)() >
    class scoped_raii_2 : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii_2<T, P1, P2, ctor, dtor> base_type;

        explicit scoped_raii_2(device_type &dev,const P1 & p1, const P2 &p2)
        : _M_device(dev)
        {
            (_M_device.*ctor)(p1,p2);
        }

        ~scoped_raii_2()
        {
            (_M_device.*dtor)();
        }

    private:
        device_type & _M_device;
    };

    template <typename T, typename P1, typename P2, typename P3, void (T::*ctor)(P1,P2,P3), void (T::*dtor)() >
    class scoped_raii_3 : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii_3<T, P1, P2, P3, ctor, dtor> base_type;

        explicit scoped_raii_3(device_type &dev,const P1 & p1, const P2 &p2, const P3 &p3)
        : _M_device(dev)
        {
            (_M_device.*ctor)(p1,p2,p3);
        }

        ~scoped_raii_3()
        {
            (_M_device.*dtor)();
        }

    private:
        device_type & _M_device;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4, void (T::*ctor)(P1,P2,P3,P4), void (T::*dtor)() >
    class scoped_raii_4 : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii_4<T, P1, P2, P3, P4, ctor, dtor> base_type;

        explicit scoped_raii_4(device_type &dev,const P1 & p1, const P2 &p2, const P3 &p3, const P4 &p4)
        : _M_device(dev)
        {
            (_M_device.*ctor)(p1,p2,p3,p4);
        }

        ~scoped_raii_4()
        {
            (_M_device.*dtor)();
        }

    private:
        device_type & _M_device;
    };

    template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5, void (T::*ctor)(P1,P2,P3,P4,P5), void (T::*dtor)() >
    class scoped_raii_5 : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii_5<T, P1, P2, P3, P4, P5, ctor, dtor> base_type;

        explicit scoped_raii_5(device_type &dev,const P1 & p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5)
        : _M_device(dev)
        {
            (_M_device.*ctor)(p1,p2,p3,p4,p5);
        }

        ~scoped_raii_5()
        {
            (_M_device.*dtor)();
        }

    private:
        device_type & _M_device;
    };

} // namespace more

#endif /* _SCOPED_RAII_HH_ */
