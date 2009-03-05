/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ITIMER_HH_
#define _ITIMER_HH_ 

#include <sys/time.h>
#include <tr1/type_traits>

#include <pthread++.hh>

namespace more {  

    // to be declared with external linkage

    struct tick_type
    {
        volatile uint32_t current;
        posix::mutex m;
        posix::cond  c;
    };

    // threads derive privately from timer in order
    // to access to tick() and wait_for_tick() member functions.

    template <tick_type & T>
    struct timer
    {
        // the SIGALARM signal handler
        static void tick(int)
        {
            timeval now;
            ::gettimeofday(&now, NULL);

            posix::scoped_lock<posix::mutex> lock(T.m);
            T.current = now.tv_sec * 1000 + now.tv_usec / 1000;
            T.c.broadcast();
        }

        void wait_for_tick(uint32_t &last)
        {
            posix::scoped_lock<posix::mutex> lock(T.m);
            if (last == T.current)
                T.c.wait(lock);
            last = T.current;
        }
    };

    // simple iterm class
    //

    template <int w> struct itimer_trait;
    template <> struct itimer_trait<ITIMER_REAL>    : public std::tr1::true_type {};
    template <> struct itimer_trait<ITIMER_VIRTUAL> : public std::tr1::true_type {};
    template <> struct itimer_trait<ITIMER_PROF>    : public std::tr1::true_type {};

    template <int which>
    class itimer : private itimer_trait<which>
    {
    public:

        itimer()
        {}

        itimer(const struct timeval *value)
        { set(value); }

        itimer(int sec, int usec)
        {
           timeval t = { sec, usec };
           set(&t);
        }

        ~itimer()
        {}

        int get(struct itimerval *value) const
        { return ::getitimer(which,value); }

        int set(const struct itimerval * value, struct itimerval *ovalue = NULL)
        { return ::setitimer(which,value,ovalue); }

        int set(const struct timeval *value)
        {
            _M_itv.it_interval.tv_sec = value->tv_sec;
            _M_itv.it_interval.tv_usec = value->tv_usec;
            _M_itv.it_value.tv_sec = value->tv_sec;
            _M_itv.it_value.tv_usec = value->tv_usec;
           
            return ::setitimer(which, &_M_itv, NULL); 
        }

        void stop()
        { set(0,0); }

    private:
        itimerval _M_itv;

        // non-copyable idiom
        itimer(const itimer &);
        itimer & operator=(const itimer &);
    };
} // namespace more

#endif /* _ITIMER_HH_ */
