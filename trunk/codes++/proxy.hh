/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef PROXY_HH
#define PROXY_HH

#include <tr1/functional>
#include <stdexcept>
#include <typeinfo>

namespace more 
{
    // to be used in place of typedef... 
    // define USE_EXPLICIT to enforce typechecking
    //
    // usage: typedef proxy<int, 0> my_int;
    //        typedef proxy<int, 1> your_int;
    //

    template <typename T,int N = 0>
    class proxy {

        T _M_val;
    public:

#ifdef USE_EXPLICIT
        explicit 
#endif        
        proxy(T v = T())
        : _M_val(v)
        {}

        proxy(const proxy &rhs)
        { _M_val = rhs._M_val; }

        proxy & operator=(const proxy &rhs)
        { _M_val = rhs._M_val; }

        proxy & operator=(const T &val)
        { _M_val = val; }

        // exposing internals requires 
        // both const and non const methods

        T & get()
        { return _M_val; }

        const T & get() const
        { return _M_val; }

#ifndef USE_EXPLICIT
        operator const T &() const
        { return _M_val; }

        operator T &() 
        { return _M_val; }
#endif
        T *
        operator &()
        { return &_M_val; }

        const T *
        operator &() const
        { return &_M_val; }
    };

    //////////////////////////////////////
    // to detect writers at compile time
    //

    template <typename T, int N = 0>
    class write_enable
    {
        T _M_val;
    public:

        explicit 
        write_enable(T v = T())
        : _M_val(v)
        {}

        write_enable &
        operator=(const write_enable &rhs)
        { 
            _M_val = rhs._M_val; 
            return *this;
        }

        const T *
        operator &() const
        { return &_M_val; }

        operator const T &() const
        { return _M_val; }

    };

    /////////////////////////////
    // runtime lockable variable

    template <typename T>
    class lockable
    {
    public:

        explicit
        lockable(bool enable = false)
        : _M_value(),
          _M_locked(enable)
        {}

        explicit 
        lockable(const T & value, bool enable = false)
        : _M_value(value),
          _M_locked(enable)
        {}

        lockable(const lockable & rhs)
        : _M_value(rhs._M_value),
          _M_locked(rhs._M_locked)
        {}

        lockable & 
        operator=(lockable rhs)
        {
            _M_checklock();
            rhs.swap(*this);
            return *this;
        }

        lockable & 
        operator=(const T & value)
        {            
            _M_checklock();
            _M_value = value;
            return *this;
        }

        lockable &
        swap(lockable &rhs)
        {
            std::swap(_M_value, rhs._M_value);
            std::swap(_M_locked,rhs._M_locked);
        }

        /////////////////////////////////////
        // internals are available readonly!

        operator const T &() const
        { 
            return _M_value;
        }

        const T *
        operator &() const
        { 
            return & _M_value;
        }

        void lock()
        {
            _M_locked = true;
        }

        void unlock()
        {
            _M_locked = false;
        }

        bool
        is_locked() const
        {
            return _M_locked;
        }

    private:
        void
        _M_checklock()
        {
            if (_M_locked)
                throw std::logic_error( std::string(typeid(T).name()).apppend(" object locked") );
        }

        T _M_value;
        bool _M_locked;
    };

    // reference_proxy...
    //

    template <typename T, int N = 0>
    class ref_proxy {

        T   _M_storage;
        T & _M_value;

    public:
        explicit ref_proxy (const T &v)
        : _M_storage(v),
        _M_value(_M_storage)
        {}

        explicit ref_proxy (std::tr1::reference_wrapper<T> pv)
        : _M_storage(),
        _M_value(pv.get())
        {}

        ref_proxy(const ref_proxy &rhs)
        : _M_storage(rhs._M_value),
        _M_value(_M_storage)
        {}

        ref_proxy & operator=(const ref_proxy &rhs)
        { _M_value = rhs._M_value; }

        ref_proxy & operator=(T val)
        { _M_value = val; }

        T & get()
        { return _M_value; }

        const T & get() const
        { return _M_value; }

    };
}

#endif /* PROXY_HH */
