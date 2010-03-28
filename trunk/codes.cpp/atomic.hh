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
#include <tr1_type_traits.hh>   // more!
#include <tr1_memory.hh>        // more!

#include <iostream>

namespace more {

    ////////////////////////////////////////////////
    // raii idiom that implements a scoped counter.

    template <typename T>
    class scoped_counter
    {
    public:

        scoped_counter(T &ref)
        : _M_ref(ref)
        {
            _M_previous = ref++;
        }

        ~scoped_counter()
        {
            _M_ref--;
        }

        T previous() const
        {
            return _M_previous;
        }

        T
        get() const
        {
            return _M_ref;
        }

    private:

        T _M_previous;
        T  & _M_ref;

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
    class atomic : atomic_enabled_for< std::is_integral<T>::value || std::is_pointer<T>::value >
    {
    public:

        explicit atomic(T v=T()) 
        : _M_value(v)
        {}

        atomic(const atomic &value)
        : _M_value(value._M_value)
        {}

        atomic &
        operator=(const atomic &rhs)
        {
            _M_value = rhs._M_value;
            return *this;            
        }

        atomic &
        operator=(const T &value)
        {
            _M_value = value;
            return *this;            
        }

        ~atomic()
        {}

        //////////////////////////////////////////////////
        // full atomic swap is not supported by the arch.

        T
        swap(T val) 
        { return __sync_lock_test_and_set(&_M_value, val); }


        operator T() const 
        { return _M_value; }

#define __SYNC(builtin) const T builtin(T val) { return __sync ## builtin(&_M_value, val); }

        T 
        operator++(int) 
        { return __sync_fetch_and_add(&_M_value, 1); }        

        T 
        operator--(int) 
        { return __sync_fetch_and_sub(&_M_value, 1); }

        __SYNC(fetch_and_or);
        __SYNC(fetch_and_and);
        __SYNC(fetch_and_xor);
        __SYNC(fetch_and_nand);

        T 
        operator++() 
        { return  __sync_add_and_fetch(&_M_value, 1); }        

        T 
        operator--() 
        { return  __sync_sub_and_fetch(&_M_value, 1); }

        T
        operator &=(T v) 
        { return  __sync_and_and_fetch(&_M_value,v); }

        T 
        operator |=(T v) 
        { return __sync_or_and_fetch(&_M_value,v); }

        T 
        operator ^=(T v) 
        { return  __sync_xor_and_fetch(&_M_value,v); }        

        __SYNC(nand_nad_fetch);

        T 
        val_compare_and_swap(T oldval, T newval) 
        { return  __sync_val_compare_and_swap(&_M_value, oldval, newval); }
 
        bool 
        bool_compare_and_swap(T oldval, T newval) 
        { return __sync_bool_compare_and_swap(&_M_value, oldval, newval); }

        static void 
        memory_barrier()
        { 
            __sync_synchronize(); 
        }

    private: 
        T _M_value;
    };

    // the following atomic_ptr is based on an idea of Alexandrescu, 
    // see http://www.ddj.com/cpp/184403766 for further details.

    template <typename Atomicity = atomicity::DEFAULT>
    struct atomic_class 
    {
        typename Atomicity::mutex _M_mutex;

        atomic_class()
        : _M_mutex()
        {}

        virtual ~atomic_class()
        {}
    };

    template <typename T, class Atomicity = atomicity::DEFAULT>
    class atomic_ptr {

    public:
        explicit atomic_ptr(volatile T& obj)
        : _M_ptr (const_cast<T*>(&obj)), 
          _M_lock(const_cast<T*>(&obj)->_M_mutex) 
        {}

        atomic_ptr(volatile T& obj, typename Atomicity::mutex& _m)
        : _M_ptr(const_cast<T*>(&obj)), 
          _M_lock(_m) 
        { }
 
        ~atomic_ptr() 
        {}

        T& 
        operator*()
        { return *_M_ptr; }

        T* 
        operator->()
        { return _M_ptr; }

    private:

        atomic_ptr(const atomic_ptr&);              // uncopyable
        atomic_ptr& operator=(const atomic_ptr&);   // uncopyable

        T* _M_ptr;
        typename Atomicity::scoped_lock _M_lock;
    };

} // namespace more

#endif /* ATOMIC_HH */
