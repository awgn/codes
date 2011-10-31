/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 *
 * note: compile with -D_GLIBCXX_USE_NANOSLEEP -D_GLIBCXX_USE_SCHED_YIELD 
 */

#ifndef _SPINLOCK_HPP_
#define _SPINLOCK_HPP_ 

#if __GNUC__ == 4 && __GNUC_MINOR__ > 4
#include <atomic>
#else
#include <cstdatomic>
#endif

#include <thread>
#include <mutex>
#include <cassert>
#include <cstdio>

// This is a c++ implementation of the Linux Kernel "ticket spinlock".
// http://lwn.net/Articles/267968/
//

namespace more { 

    namespace policy {

        ////// spinlock policies

        struct lock_yield 
        {
            enum { threshold = 0 };
            static void wait(int,int,int)
            {
                std::this_thread::yield();
            }
        };

        template <int N>
        struct lock_usleep 
        {
            enum { threshold = 0 };
            static void wait(int,int,int)
            {
                std::this_thread::sleep_for(std::chrono::microseconds(N));
            }
        };

        struct lock_aggressive 
        {
            enum { threshold = 0 };
            static void wait(int,int,int)
            {}
        };

        template <int N = 1024>
        struct lock_backoff 
        {
            enum { threshold = N };
            static void wait(int n, int& t, int)
            {
                if ((n % t) == 0) {
                    t= t>>1 ? : 1;
                    lock_yield::wait(0,0,0);
                }
            }
        };

        template <int N = 1024>
        struct lock_smart 
        {
            enum { threshold = N };
            static void wait(int n, int &t, int d)
            {
                if (d > 1 || (n % t) == 0) { 
                    t= t>>1 ? : 1;
                    lock_yield::wait(0,0,0);
                }
            }
        };

    } // namespace details

    ///////////////////////////  spinlock...

    template <typename Policy>
    struct spinlock 
    {
        spinlock()
        : _M_value(0)
        {}

        spinlock(const spinlock &) = delete;
        spinlock& operator=(const spinlock &) = delete;

        void lock()
        {   
            int t = Policy::threshold;
            unsigned int v = 0;
            for(int n=0; _M_value.compare_exchange_strong(v, 1) == false; ++n)
            { 
                v = 0;
                Policy::wait(n,t,0);
            }
        }

        void unlock()
        {
            _M_value.store(0);
        }

    private:
        std::atomic_uint _M_value;
    };
 
    typedef spinlock<policy::lock_yield>        spinlock_yield;
    typedef spinlock<policy::lock_aggressive>   spinlock_aggressive;
    typedef spinlock<policy::lock_backoff<>>    spinlock_backoff;
    typedef spinlock<policy::lock_smart<>>      spinlock_smart;

    ///////////////////////////  ticket_spinlock...

    template <typename Policy>
    struct ticket_spinlock 
    {
        ticket_spinlock()
        : _M_ticket(0), _M_value(0)
        {}
        
        ticket_spinlock(const ticket_spinlock &) = delete;
        ticket_spinlock& operator=(const ticket_spinlock &) = delete;

        void lock()
        {             
            const unsigned int my_ticket = _M_ticket++;
            int t = Policy::threshold;
            for(int n = 1, d = 0; (d=(my_ticket-_M_value)); n++) 
            { 
                Policy::wait(n,t,d);
            }
        }

        void unlock()
        {
           ++_M_value; 
        }

    private:
        std::atomic_uint _M_ticket;        
        std::atomic_uint _M_value;        
    };

    typedef ticket_spinlock<policy::lock_yield>        ticket_spinlock_yield;
    typedef ticket_spinlock<policy::lock_aggressive>   ticket_spinlock_aggressive;
    typedef ticket_spinlock<policy::lock_backoff<>>    ticket_spinlock_backoff;
    typedef ticket_spinlock<policy::lock_smart<>>      ticket_spinlock_smart;
 
    ///////////////////////////  spinlock_open_recursive...

    template <typename Policy>
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
            if (_M_owner != std::this_thread::get_id()) {
                _M_lock.lock();
                _M_owner = std::this_thread::get_id();
            }
        }

        void unlock()
        {
            assert(_M_owner == std::this_thread::get_id());
            _M_owner = std::thread::id();
            _M_lock.unlock();
        }

    private:
        spinlock<Policy> _M_lock; 
        std::thread::id _M_owner;
    };
}

#endif /* _SPINLOCK_HPP_ */
