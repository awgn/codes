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

#include <iostream>
#include <atomicity-policy.hh>

#if   __GNUC__ >= 4
#include <tr1/memory>
#elif (__GNUC__ == 3) && (__GNUC_MINOR__ == 4)
#include <bits/atomicity.h>
#else
#error "this compiler is not supported"
#endif

namespace more {

#ifdef USE_X86_FEATURE_XMM2
    static inline
    void mb()   { asm volatile("mfence" ::: "memory"); }
    static inline
    void rmb()  { asm volatile("lfence" ::: "memory"); }
    static inline
    void wmb()  { asm volatile("sfence" ::: "memory"); }
#else
#warning "compile with -DUSE_X86_FEATURE_XMM2 to have optimized memory barriers"
    static inline
    void mb()  { asm volatile("lock; addl $0,0(%%esp)" ::: "memory"); }
    static inline
    void rmb() { asm volatile("lock; addl $0,0(%%esp)" ::: "memory"); }
    static inline
    void wmb() { asm volatile("lock; addl $0,0(%%esp)" ::: "memory"); }
#endif

    template <typename T = _Atomic_word>
    class atomic_word
    {
    public:
        explicit atomic_word(T v=T()) 
        : _M_value(v)
        {}

        const T 
        operator=(T n) volatile 
        { return _M_value = n; }

        operator T() const volatile 
        { return _M_value; }

#ifdef USE_GCC_BUILTIN
#define __SYNC(builtin) const T builtin(T val) volatile { return __sync ## builtin(&_M_value, val); }

        const T 
        operator++(int) volatile
        { return __sync_fetch_and_add(&_M_value, 1); }        
        const T 
        operator--(int) volatile
        { return __sync_fetch_and_sub(&_M_value, 1); }

        __SYNC(fetch_and_or);
        __SYNC(fetch_and_and);
        __SYNC(fetch_and_xor);
        __SYNC(fetch_and_nand);

        const T 
        operator++() volatile 
        { return  __sync_add_and_fetch(&_M_value, 1); }        
        const T 
        operator--() volatile
        { return  __sync_sub_and_fetch(&_M_value, 1); }
        const T
        operator &=(T v) volatile
        { return  __sync_and_and_fetch(&_M_value,v); }
        const T 
        operator |=(T v) volatile 
        { return __sync_or_and_fetch(&_M_value,v); }
        const T 
        operator ^=(T v) volatile 
        { return  __sync_xor_and_fetch(&_M_value,v); }        

        __SYNC(nand_nad_fetch);

        const T 
        val_compare_and_swap(T oldval, T newval) volatile
        { return  __sync_val_compare_and_swap(&_M_value, oldval, newval); }
 
        bool 
        bool_compare_and_swap(T oldval, T newval) volatile
        { return __sync_bool_compare_and_swap(&_M_value, oldval, newval); }

        const T
        swap(T val) volatile
        { return __sync_lock_test_and_set(&_M_value, val); }

        static void memory_barrier()
        { __sync_synchronize(); }

#undef __SYNC
#else // USE__GNU_CXX
        _Atomic_word operator++(int) volatile
        { return __gnu_cxx::__exchange_and_add(&_M_value,1); }

        _Atomic_word operator--(int) volatile
        { return __gnu_cxx::__exchange_and_add(&_M_value,-1); } 
#endif

    private: 
        volatile T _M_value;
    };

    // the following volatile_ptr is based on an idea of Alexandrescu, 
    // see http://www.ddj.com/cpp/184403766 for further details.

    template <typename Atomicity = atomicity::GNU_CXX>
    struct atomic_class 
    {
        typename Atomicity::mutex _M_mutex;

        atomic_class()
        : _M_mutex()
        {}

        virtual ~atomic_class()
        {}
    };

    template <typename T, class Atomicity = atomicity::GNU_CXX>
    class volatile_ptr {

    public:
        explicit volatile_ptr(volatile T& obj)
        : _M_ptr (const_cast<T*>(&obj)), 
          _M_lock(const_cast<T*>(&obj)->_M_mutex) 
        {}

        volatile_ptr(volatile T& obj, typename Atomicity::mutex& _m)
        : _M_ptr(const_cast<T*>(&obj)), 
          _M_lock(_m) 
        { }
 
        ~volatile_ptr() 
        {}

        T& 
        operator*()
        { return *_M_ptr; }

        T* 
        operator->()
        { return _M_ptr; }

    private:

        volatile_ptr(const volatile_ptr&);              // uncopyable
        volatile_ptr& operator=(const volatile_ptr&);   // uncopyable

        T* _M_ptr;
        typename Atomicity::scoped_lock _M_lock;
    };

    template <typename Atomicity>
    class volatile_ptr<_Atomic_word,Atomicity>;       // use atomic_word class directly. 

} // namespace more

#endif /* ATOMIC_HH */
