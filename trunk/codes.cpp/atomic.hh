/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef ATOMIC_HH
#define ATOMIC_HH

#include <atomicity-policy.hh>  // more!

#include <tr1/type_traits>   
#include <tr1/memory>        
#include <iostream>

namespace more {

    ////////////////////////////////////////////////
    // raii idiom that implements a scoped counter.

    template <typename T>
    class scoped_counter
    {
    public:

        scoped_counter(T &ref)
        : m_ref(ref)
        {
            m_previous = ref++;
        }

        ~scoped_counter()
        {
            m_ref--;
        }

        T previous() const
        {
            return m_previous;
        }

        T
        get() const
        {
            return m_ref;
        }

    private:

        T m_previous;
        T  & m_ref;

        // non-copyable idiom
        scoped_counter(const scoped_counter &);
        scoped_counter & operator=(const scoped_counter &);
    };

    /////////////////////////
    // atomic template class

    template <bool N> struct atomic_enabled_for;
    template <>
    struct atomic_enabled_for<true>
    {};

    template <typename T>
    class atomic : atomic_enabled_for< std::tr1::is_integral<T>::value || std::tr1::is_pointer<T>::value >
    {
    public:

        explicit atomic(T v=T()) 
        : m_value(v)
        {}

        atomic(const atomic &value)
        : m_value(value.m_value)
        {}

        atomic &
        operator=(const atomic &rhs)
        {
            m_value = rhs.m_value;
            return *this;            
        }

        atomic &
        operator=(const T &value)
        {
            m_value = value;
            return *this;            
        }

        ~atomic()
        {}

        //////////////////////////////////////////////////
        // full atomic swap is not supported by the arch.

        T
        swap(T val) 
        { return __sync_lock_test_and_set(&m_value, val); }


        operator T() const volatile 
        { return m_value; }

#define __SYNC(builtin) T builtin(T val) volatile { return __sync_ ## builtin(&m_value, val); }

        T 
        operator+=(T value) volatile 
        { return  __sync_add_and_fetch(&m_value, value); }        

        T 
        operator-=(T value) volatile
        { return  __sync_sub_and_fetch(&m_value, value); }


        T 
        operator++(int) volatile 
        { return __sync_fetch_and_add(&m_value, 1); }        

        T 
        operator--(int) volatile
        { return __sync_fetch_and_sub(&m_value, 1); }

        T 
        operator++() volatile 
        { return  __sync_add_and_fetch(&m_value, 1); }        

        T 
        operator--() volatile
        { return  __sync_sub_and_fetch(&m_value, 1); }

        __SYNC(fetch_and_or);
        __SYNC(fetch_and_and);
        __SYNC(fetch_and_xor);
        __SYNC(fetch_and_nand);

        __SYNC(or_and_fetch);
        __SYNC(and_and_fetch);
        __SYNC(xor_and_fetch);
        __SYNC(nnand_and_fetch);

        __SYNC(lock_test_and_set);

        void
        lock_release() volatile
        {
            __sync_lock_release(&m_value);
        }

        T
        operator &=(T v) volatile
        { return  __sync_and_and_fetch(&m_value,v); }

        T 
        operator |=(T v) volatile
        { return __sync_or_and_fetch(&m_value,v); }

        T 
        operator ^=(T v) volatile
        { return  __sync_xor_and_fetch(&m_value,v); }        

        __SYNC(nand_nad_fetch);

        T 
        val_compare_and_swap(T oldval, T newval) volatile 
        { return  __sync_val_compare_and_swap(&m_value, oldval, newval); }
 
        bool 
        bool_compare_and_swap(T oldval, T newval) volatile
        { return __sync_bool_compare_and_swap(&m_value, oldval, newval); }

        static void 
        memory_barrier() 
        { 
            __sync_synchronize(); 
        }

    private: 
        T m_value;
    };

    // the following atomic_ptr is based on an idea of Alexandrescu, 
    // see http://www.ddj.com/cpp/184403766 for further details.

    template <typename Atomicity = atomicity::DEFAULT>
    struct atomic_class 
    {
        typename Atomicity::mutex m_mutex;

        atomic_class()
        : m_mutex()
        {}

        virtual ~atomic_class()
        {}
    };

    template <typename T, class Atomicity = atomicity::DEFAULT>
    class atomic_ptr {

    public:
        explicit atomic_ptr(volatile T& obj)
        : m_ptr (const_cast<T*>(&obj)), 
          m_lock(const_cast<T*>(&obj)->m_mutex) 
        {}

        atomic_ptr(volatile T& obj, typename Atomicity::mutex& _m)
        : m_ptr(const_cast<T*>(&obj)), 
          m_lock(_m) 
        { }
 
        ~atomic_ptr() 
        {}

        T& 
        operator*()
        { return *m_ptr; }

        T* 
        operator->()
        { return m_ptr; }

    private:

        atomic_ptr(const atomic_ptr&);              // uncopyable
        atomic_ptr& operator=(const atomic_ptr&);   // uncopyable

        T* m_ptr;
        typename Atomicity::scoped_lock m_lock;
    };

} // namespace more

#endif /* ATOMIC_HH */
