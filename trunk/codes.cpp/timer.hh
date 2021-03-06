/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TIMER_HH_
#define _TIMER_HH_ 

#include <sys/time.h>

#include <pthread++.hh>         // more!
#include <static_assert.hh>     // more!
#include <system_error.hh>      // more!

#include <tr1/type_traits>   
#include <tr1/array>         

#include <cstring>
#include <string>

namespace more { namespace time {

    struct tick_data
    {
        typedef struct {        
            posix::mutex _mutex;
            posix::cond  _cond;
        } tick_data_type;

        std::tr1::array<tick_data_type, 64 /* under linux SIGRTMAX is a function and cannot appear in constant expressions */ +1 > thread_data; 
    };

    ////////////////////////////////////////////
    // ... to be declared with external linkage

    struct tick_type : public tick_data
    {
        volatile uint32_t current;
    };

    //////////////////////////////////////////////////////////////////////
    // threads listening to tick derive privately from enable_tick class 
    // in order to inherit wait_for_tick() member function.

    template <tick_type & T, int SIG>
    struct enable_tick
    {
        void wait_for_tick(uint32_t &last)
        {
            posix::scoped_lock<posix::mutex> lock(T.thread_data[SIG]._mutex);

            if (last == T.current) {
                T.thread_data[SIG]._cond.wait(lock);
                while (last == T.current) {
                    // avoid spurious wakeup which do not return EINTR but 0 according to POSIX.
                    T.thread_data[SIG]._cond.wait(lock);
                }
            }

            last = T.current;
        }
    };

    ////////////////////////////////////////////////////////////////////////
    // itimer and itimer_pulse_thread 

    template <int w> struct itimer_trait;
    template <> struct itimer_trait<ITIMER_REAL>
    {
        enum { signal_expiration = SIGALRM };
    };

    template <> struct itimer_trait<ITIMER_VIRTUAL> 
    {
        enum { signal_expiration = SIGVTALRM };
    };

    template <> struct itimer_trait<ITIMER_PROF> 
    {
        enum { signal_expiration = SIGPROF };
    };

    //////////////////
    // itimer object

    template <int WHICH>
    class itimer
    {
    public:

        itimer()
        : m_itv(), m_itv_p(NULL)
        {}

        explicit itimer(const itimerval * value, itimerval *ovalue = NULL)
        : m_itv(*value), m_itv_p(ovalue)
        {}

        explicit itimer(const timeval *value)
        : m_itv(), m_itv_p(0)
        {
            this->set(value);
        }

        itimer(time_t sec, suseconds_t usec)
        : m_itv(), m_itv_p(0)
        {
            this->set(sec,usec);
        }

        ~itimer()
        {
            // stop the itimer
            itimerval stop = {{0, 0}, {0, 0}};
            ::setitimer(WHICH, &stop, 0);
        }
        
        int start()
        {
            // start the itimer
            return ::setitimer(WHICH, &m_itv, m_itv_p);
        }

        void set(const itimerval * value, itimerval *ovalue = NULL)
        { 
            m_itv = * value; 
            m_itv_p = ovalue;
        }

        void set(const struct timeval *value)
        {
            m_itv.it_interval.tv_sec  = value->tv_sec;
            m_itv.it_interval.tv_usec = value->tv_usec;
            m_itv.it_value.tv_sec     = value->tv_sec;
            m_itv.it_value.tv_usec    = value->tv_usec;
        }

        void set(time_t sec, suseconds_t usec)
        {
            m_itv.it_interval.tv_sec  = sec;
            m_itv.it_interval.tv_usec = usec;
            m_itv.it_value.tv_sec     = sec;
            m_itv.it_value.tv_usec    = usec;
        }

        int get(struct itimerval *value) const
        { 
            return ::getitimer(WHICH,value); 
        }

        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT, Traits> &out, const itimer &rhs)
        {
            out << "itimerval:{ value:{" << rhs.m_its.it_value.tv_sec  << ',' << 
                                rhs.m_its.it_value.tv_usec << 
                         "} interval:{" << rhs.m_its.it_interval.tv_sec << ',' <<
                                           rhs.m_its.it_interval.tv_usec <<  "} }"; 
            return out;
        }


    private: 
        itimerval   m_itv;
        itimerval  *m_itv_p;
    };


    template <int WHICH, tick_type & T>
    class itimer_pulse_thread : public posix::thread 
    {

    public:
        itimer_pulse_thread()
        : m_itimer()
        {}

        explicit itimer_pulse_thread(const struct itimerval *value, struct itimerval *oldvalue = NULL)
        : m_itimer(value, oldvalue)
        {}

        explicit itimer_pulse_thread(const struct timeval *value)
        : m_itimer(value)
        {}

        itimer_pulse_thread(time_t sec, suseconds_t usec)
        : m_itimer(sec, usec)
        {}

        ~itimer_pulse_thread()
        {}

        void *operator()()
        {
            int sig;

            // start the timer
            if ( m_itimer.start() < 0 )
                throw more::system_error(std::string("setitimer",errno));

            sigset_t sigexp; 
            sigemptyset(&sigexp);
            sigaddset(&sigexp, itimer_trait<WHICH>::signal_expiration);

            for(timeval now;;) 
            {
                sigwait(&sigexp, &sig);
                assert( sig == itimer_trait<WHICH>::signal_expiration );
                ::gettimeofday(&now, NULL);

                posix::scoped_lock<posix::mutex> lock(T.thread_data[ itimer_trait<WHICH>::signal_expiration  ]._mutex);

                T.current = now.tv_sec * 1000 + now.tv_usec / 1000;
                T.thread_data[ itimer_trait<WHICH>::signal_expiration ]._cond.broadcast();
            }
            return static_cast<void *>(0);        
        }

    private:

        itimer<WHICH>  m_itimer;

        // non-copyable idiom
        itimer_pulse_thread(const itimer_pulse_thread &);
        itimer_pulse_thread & operator=(const itimer_pulse_thread &);
    };

    //////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////// REALTIME ////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////

    static const int SIGRT_MIN = 34;
    static const int SIGRT_MAX = 64;
    
    template <clockid_t CID, int SIGEV = SIGEV_SIGNAL, int SIGNO = SIGRT_MAX, int FLAGS = 0> 
    //
    // CID   = CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_PROCESS_CPUTIME_ID, CLOCK_THREAD_CPUTIME_ID
    // SIGEV = SIGEV_SIGNAL, SIGEV_NONE, SIGEV_THREAD, SIGEV_THREAD_ID 
    // FLAGS = 0, TIMER_ABSTIME  
    //
    class rt_timer        
    { 
    public:

        rt_timer()
        : m_id(), m_its(), m_its_p(0)
        {
            this->init();
        }

        explicit rt_timer(const itimerspec *value, itimerspec *oldvalue = NULL)
        : m_id(), m_its(*value), m_its_p(oldvalue)
        {
            this->init();
        }

        explicit rt_timer(const timespec *value)
        : m_id(), m_its(), m_its_p(0)
        {
            this->set(value);
            this->init();
        }

        rt_timer(time_t sec, long nsec)
        : m_id(), m_its(), m_its_p(0)
        {
            this->set(sec,nsec);
            this->init();
        }

        ~rt_timer()
        {
            // delete the timer
            ::timer_delete(m_id); 
        }

        int start()
        {
            // start the timer
            return ::timer_settime(m_id, FLAGS, &m_its, m_its_p);
        }

        timer_t
        get_id() const
        { return m_id; }

        void set(const itimerspec *value, itimerspec *oldvalue = NULL)
        {
            m_its   = *value;
            m_its_p = oldvalue; 
        } 

        void set(const timespec *value)
        {            
            m_its.it_interval.tv_sec  = value->tv_sec;
            m_its.it_interval.tv_nsec = value->tv_nsec; 
            m_its.it_value.tv_sec     = value->tv_sec;
            m_its.it_value.tv_nsec    = value->tv_nsec;
 
            m_its_p = NULL; 
        } 
        void set(time_t sec, long nsec)
        {            
            m_its.it_interval.tv_sec  = sec;
            m_its.it_interval.tv_nsec = nsec; 
            m_its.it_value.tv_sec     = sec;
            m_its.it_value.tv_nsec    = nsec;

            m_its_p = NULL; 
        } 

        int get(itimerspec &value) const
        {
            return ::timer_gettime(m_id, &value);
        }

        int overrun() const
        {
            return ::timer_getoverrun(m_id);
        }

    private:
        void init()
        {
            
            static_assert( CID == CLOCK_REALTIME            ||
                           CID == CLOCK_MONOTONIC           ||
                           CID == CLOCK_PROCESS_CPUTIME_ID  ||
                           CID == CLOCK_THREAD_CPUTIME_ID,  clokid_not_allowed);

            static_assert( SIGEV == SIGEV_SIGNAL ||
                           SIGEV == SIGEV_NONE   ||
                           SIGEV == SIGEV_THREAD ||
                           SIGEV == SIGEV_THREAD_ID , sigev_notify_not_allowed);

            // The  Linux  kernel  supports a range of 32 different real-time signals,
            // numbered 33 to 64. However, the  glibc  POSIX  threads  implementation
            // internally  uses  two  (for NPTL) or three (for LinuxThreads) real-time
            // signals (see pthreads(7)), and adjusts the value of  SIGRTMIN  suitably
            // (to 34 or 35)

            static_assert( SIGNO >= SIGRT_MIN && SIGNO <= SIGRT_MAX , rt_sig_numer_not_allowed); 
            static_assert( FLAGS == 0 || FLAGS == 1, timer_settime_flags_not_allowed);

            struct sigevent ev;
            ev.sigev_notify = SIGEV;
            ev.sigev_signo  = SIGNO;
            ev.sigev_value.sival_ptr = &m_id;

            if ( ::timer_create(CID, &ev, &m_id) <  0 )
                throw more::system_error(std::string("timer",errno));
        }

        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT,Traits> &out, const rt_timer &rhs)
        {
            out << "itimerspec:{ value:{" << rhs.m_its.it_value.tv_sec  << ',' << 
                                rhs.m_its.it_value.tv_nsec << 
                         "} interval:{" << rhs.m_its.it_interval.tv_sec << ',' <<
                                           rhs.m_its.it_interval.tv_nsec <<  "} }"; 
            return out;
        }

        timer_t      m_id;

    public:

        itimerspec   m_its;
        itimerspec * m_its_p;

        // non-copyable idiom
        rt_timer(const rt_timer &);
        rt_timer & operator=(const rt_timer &);

    };

    template <int SIGNO, tick_type & T, clockid_t CID = CLOCK_REALTIME >
    class rt_timer_pulse_thread : public posix::thread
    {        
    public:

        rt_timer_pulse_thread()
        : m_rt_timer(), m_update(false)
        {}

        explicit rt_timer_pulse_thread(const itimerspec *value, itimerspec *oldvalue = NULL)
        : m_rt_timer(value, oldvalue), m_update(false)
        {}

        explicit rt_timer_pulse_thread(const timespec *value)
        : m_rt_timer(value), m_update(false)
        {}

        rt_timer_pulse_thread(time_t sec, long nsec)
        : m_rt_timer(sec,nsec), m_update(false)
        {}

        ~rt_timer_pulse_thread()
        {}

        void *operator()()
        {
            int sig;

            // start the timer
            if ( m_rt_timer.start() < 0)
                throw more::system_error(std::string("timer_settime"));

            sigset_t sigexp;
            sigemptyset(&sigexp);
            sigaddset(&sigexp, SIGNO);

            for(timeval now;;)
            {
                sigwait(&sigexp, &sig);

                if (m_update) {
                    posix::scoped_lock<posix::mutex> _L_(m_mutex_update);
                        
                    if (m_rt_timer.start() < 0)
                        throw more::system_error(std::string("timer_settime"));

                    m_update = false;
                }

                assert( sig == SIGNO );
                ::gettimeofday(&now, NULL);
        
                posix::scoped_lock<posix::mutex> lock(T.thread_data[SIGNO]._mutex);

                T.current = now.tv_sec * 1000 + now.tv_usec / 1000;
                T.thread_data[SIGNO]._cond.broadcast();
            }
            return static_cast<void *>(0);
        }

        void update(const itimerspec *value)
        {
            posix::scoped_lock<posix::mutex> _L_(m_mutex_update);
            m_rt_timer.set(value, NULL);
            m_update = true; 
        } 

        void update(const timespec *value)
        {    
            posix::scoped_lock<posix::mutex> _L_(m_mutex_update);
            m_rt_timer.set(value);            
            m_update = true; 
        } 

        void update(time_t sec, long nsec)
        {  
            posix::scoped_lock<posix::mutex> _L_(m_mutex_update);
            m_rt_timer.set(sec,nsec);         
            m_update = true; 
        } 

    private:
        rt_timer<CID, SIGEV_SIGNAL, SIGNO>  m_rt_timer;        

        posix::mutex m_mutex_update;
        bool m_update;

        // non-copyable idiom
        rt_timer_pulse_thread(const rt_timer_pulse_thread &);
        rt_timer_pulse_thread & operator=(const rt_timer_pulse_thread &);
    };

} // namespace time
} // namespace more

#endif /* _TIMER_HH_ */
