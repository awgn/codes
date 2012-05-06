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
#include <string>
#include <algorithm>

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

        T m_val;
    public:

#ifdef USE_EXPLICIT
        explicit 
#endif        
        proxy(T v = T())
        : m_val(v)
        {}

        proxy(const proxy &rhs)
        { m_val = rhs.m_val; }

        proxy & operator=(const proxy &rhs)
        { m_val = rhs.m_val; }

        proxy & operator=(const T &val)
        { m_val = val; }

        // exposing internals requires 
        // both const and non const methods

        T & get()
        { return m_val; }

        const T & get() const
        { return m_val; }

#ifndef USE_EXPLICIT
        operator const T &() const
        { return m_val; }

        operator T &() 
        { return m_val; }
#endif
        T *
        operator &()
        { return &m_val; }

        const T *
        operator &() const
        { return &m_val; }
    };

    //////////////////////////////////////
    // to detect writers at compile time
    //

    template <typename T, int N = 0>
    class write_enable
    {
        T m_val;
    public:

        explicit 
        write_enable(T v = T())
        : m_val(v)
        {}

        write_enable &
        operator=(const write_enable &rhs)
        { 
            m_val = rhs.m_val; 
            return *this;
        }

        const T *
        operator &() const
        { return &m_val; }

        operator const T &() const
        { return m_val; }

    };

    /////////////////////////////
    // runtime lockable variable

    template <typename T>
    class lockable
    {
    public:

        explicit
        lockable(bool enable = false)
        : m_value(),
          m_locked(enable)
        {}

        explicit 
        lockable(const T & value, bool enable = false)
        : m_value(value),
          m_locked(enable)
        {}

        lockable(const lockable & rhs)
        : m_value(rhs.m_value),
          m_locked(rhs.m_locked)
        {}

        lockable & 
        operator=(lockable rhs)
        {
            m_checklock();
            rhs.swap(*this);
            return *this;
        }

        lockable & 
        operator=(const T & value)
        {            
            m_checklock();
            m_value = value;
            return *this;
        }

        lockable &
        swap(lockable &rhs)
        {
            std::swap(m_value, rhs.m_value);
            std::swap(m_locked,rhs.m_locked);
        }

        /////////////////////////////////////
        // internals are available readonly!

        operator const T &() const
        { 
            return m_value;
        }

        const T *
        operator &() const
        { 
            return & m_value;
        }

        void lock()
        {
            m_locked = true;
        }

        void unlock()
        {
            m_locked = false;
        }

        bool
        is_locked() const
        {
            return m_locked;
        }

    private:
        void
        m_checklock()
        {
            if (m_locked)
                throw std::logic_error( std::string(typeid(T).name()).append(" object locked") );
        }

        T m_value;
        bool m_locked;
    };

    // reference_proxy...
    //

    template <typename T, int N = 0>
    class ref_proxy {

        T   m_storage;
        T & m_value;

    public:
        explicit ref_proxy (const T &v)
        : m_storage(v),
        m_value(m_storage)
        {}

        explicit ref_proxy (std::tr1::reference_wrapper<T> pv)
        : m_storage(),
        m_value(pv.get())
        {}

        ref_proxy(const ref_proxy &rhs)
        : m_storage(rhs.m_value),
        m_value(m_storage)
        {}

        ref_proxy & operator=(const ref_proxy &rhs)
        { m_value = rhs.m_value; }

        ref_proxy & operator=(T val)
        { m_value = val; }

        T & get()
        { return m_value; }

        const T & get() const
        { return m_value; }

    };
}

#endif /* PROXY_HH */
