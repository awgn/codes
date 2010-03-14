/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TSPINLOCK_HH_
#define _TSPINLOCK_HH_ 

#include <sched.h>
#include <pthread.h>

#include <iostream>
#include <cassert>


// This is a c++ implementation of the Linux Kernel "ticket spinlock".
// http://lwn.net/Articles/267968/
//

namespace more { 

    struct tspinlock 
    {
        tspinlock()
        : _M_ticket(0), _M_value(0)
        {}

        void lock()
        { this->lock(_M_ticket,_M_value); }

        void unlock()
        { this->unlock(_M_ticket,_M_value); }

        ////////////////
        // static utils

        static 
        void lock(volatile int &t, volatile int &v)
        {
            int ticket = __sync_fetch_and_add(&t, 1);
            busywait:
            if ( v != ticket) {
                sched_yield();
                goto busywait;
            }
        }

        static 
        void unlock(volatile int &t, volatile int &v)
        { __sync_add_and_fetch(&v, 1); }

    private:
        volatile int _M_ticket;        
        volatile int _M_value;        
    };


    struct tspinlock_half_recursive
    {
        tspinlock_half_recursive()
        : _M_lock(),
          _M_owner()
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
        tspinlock _M_lock; 
        pthread_t _M_owner;
    };


    struct scoped_tspinlock {
       
        // scoped tlock
        //

        scoped_tspinlock(tspinlock &sl)
        : _M_sl(sl)
        {
            _M_sl.lock();
        }

        ~scoped_tspinlock()
        {
            _M_sl.unlock();
        }

    private:
        tspinlock &_M_sl;

        scoped_tspinlock(const scoped_tspinlock &);
        scoped_tspinlock & operator=(const scoped_tspinlock &);
        void * operator new(std::size_t);
        void * operator new[](std::size_t);
        void operator delete(void*);
        void operator delete[](void*);
        scoped_tspinlock* operator&();
 
    };
}

#endif /* _TSPINLOCK_HH_ */
