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

    template <typename D, typename T, typename R, R (T::*acquire)(), void (T::*release)() > 
    class scoped_raii : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii<D, T, R, acquire, release> base_type;

        explicit scoped_raii(device_type &dev)
        : _M_device(dev),
          _M_value()
        {
            _M_value = (_M_device.*acquire)();
        }

        virtual ~scoped_raii()
        {
            if (static_cast<D &>(*this).release_cond())
                (_M_device.*release)();
        }

        operator R()
        {
            return _M_value;
        }

    protected: 

        device_type & _M_device;
        R _M_value; 
    };

    template <typename D, typename T, void (T::*acquire)(), void (T::*release)()>
    class scoped_raii<D,T,void,acquire,release> : private noncopyable
    {
    public:
        typedef T device_type;
        typedef scoped_raii<D, T, void, acquire, release> base_type;

        explicit scoped_raii(device_type &dev)
        : _M_device(dev)
        {
            (_M_device.*acquire)();
        }

        ~scoped_raii()
        {
            (_M_device.*release)();
        }

    private:
        device_type & _M_device;
         
    };

    ////////////// generic scoped_lock / scoped_try_lock...

    namespace raii {

        template <typename T>
        struct scoped_lock : public more::scoped_raii< scoped_lock<T>, T, void, &T::lock, &T::unlock>
        {
            scoped_lock(T &dev)
            : scoped_lock<T>::base_type(dev)
            {}
        };

        template <typename T>
        struct scoped_try_lock : public more::scoped_raii<scoped_try_lock<T>, T, bool, &T::try_lock, &T::unlock>
        {
            scoped_try_lock(T &dev)
            : scoped_try_lock<T>::base_type(dev)
            {}

            bool release_cond()
            {
                // *this is convertible to bool
                return *this;
            }
        };

    } // namespace raii

} // namespace more

#endif /* _SCOPED_RAII_HH_ */
