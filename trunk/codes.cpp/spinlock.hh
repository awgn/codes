/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SPINLOCK_HH_
#define _SPINLOCK_HH_ 

#include <sched.h>
#include <pthread.h>

#include <noncopyable.hh>   // more!
#include <atomic.hh>        // more!

#include <iostream>
#include <cassert>
#include <cstdio>

// This is a c++ implementation of the Linux Kernel "ticket spinlock".
// http://lwn.net/Articles/267968/
//

namespace more { 

    struct lock_relaxed 
    {
        static void wait(int)
        {
            sched_yield();
        }
    };

    struct lock_aggressive 
    {
        static void wait(int)
        {}
    };

    template <int N = 10>
    struct lock_smart 
    {
        static void wait(int n)
        {
            if (n > N)
                lock_relaxed::wait(0);
        }
    };

    template <typename Policy>
    struct spinlock 
    {
        spinlock()
        : _M_ticket(0), _M_value(0)
        {}

        void lock()
        {             
            const unsigned my_ticket = _M_ticket++;
            for(int n = 1; _M_value != my_ticket; n++) 
            {
                Policy::wait(n);
            }
        }

        void unlock()
        {
           ++_M_value; 
        }

    private:
        more::atomic<unsigned int> _M_ticket;        
        more::atomic<unsigned int> _M_value;        
    };

    class spinlock_open_recursive
    {
    public:
        
        spinlock_open_recursive()
        : _M_lock(),
          _M_owner()
        {}

        ~spinlock_open_recursive()
        {}

        void lock()
        {
            if (_M_owner != pthread_self()) {
                _M_lock.lock();
                _M_owner = pthread_self();
            }
        }

        void unlock()
        {
            assert(_M_owner == pthread_self());
            _M_owner = 0;
            _M_lock.unlock();
        }

    private:
        spinlock< lock_smart<10> > _M_lock; 
        pthread_t _M_owner;
    };

    template <typename Tp>
    struct scoped_spinlock : private more::noncopyable 
    {
        // scoped lock... (RAII) 

        scoped_spinlock(spinlock<Tp> &lock)
        : _M_lock(lock)
        {
            _M_lock.lock();
        }

        ~scoped_spinlock()
        {
            _M_lock.unlock();
        }

    private:
        spinlock<Tp> &_M_lock;
    };
}

#endif /* _SPINLOCK_HH_ */
