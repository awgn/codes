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

#include <iostream>
#include <cassert>

#include <sched.h>
#include <pthread.h>

// This is a c++ implementation of the Linux Kernel "ticket spinlock".
// http://lwn.net/Articles/267968/
//

namespace more { 

    struct tspinlock 
    {
        tspinlock()
        : _M_value(0)
        {}

        void lock()
        { lock(_M_value); }

        void unlock()
        { unlock(_M_value); }

        ////////////////
        // static utils

        static 
        void lock(volatile int &sl)
        {
            int spinlock = __sync_fetch_and_add(&sl, 1L << 16);
            unsigned short ticket = (spinlock >> 16) & 0xffff;
            busywait:
            if ( (sl & 0xffff )  != ticket) {
                // sched_yield();
                usleep(10);
                goto busywait;
            }
        }

        static 
        void unlock(volatile int &sl)
        {
            unsigned int spinlock = __sync_add_and_fetch(&sl, 1);
            assert(sl == spinlock);
            if ( ! (spinlock & 0xffff) ) {
                __sync_sub_and_fetch(&sl, 1<<16);
            }    
        }

        const int __value() const
        { return _M_value; }

    private:
        volatile int _M_value;        
    };


    struct tspinlock_recursive
    {
        tspinlock_recursive()
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
