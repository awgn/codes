/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef PTHREADPP_HH
#define PTHREADPP_HH

#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>

#include <system_error.hh>  // more!
#include <noncopyable.hh>   // more!
#include <static_assert.hh> // more!

#include <tr1/type_traits> 
#include <tr1/functional>  
#include <tr1/memory>      

#include <memory>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <set>

namespace more { namespace posix 
{    
    using namespace std::tr1::placeholders;
    using std::tr1::shared_ptr;
    using std::tr1::mem_fn;
    using std::tr1::bind;

    namespace  
    {
        template <int n>
        struct int2type {
            enum { value = n };
        };

        template <typename T>
        struct type2type {
            typedef T type;
        };
    }

    class thread_attr
    {
    public:
        thread_attr()
        : m_value()
        { ::pthread_attr_init(&m_value); }

        ~thread_attr()
        { /* ::pthread_attr_destroy(&m_value); */ }

        int setdetachstate(int detachstate)
        { return ::pthread_attr_setdetachstate(&m_value, detachstate); }

        int getdetachstate(int *detachstate) const
        { return ::pthread_attr_getdetachstate(&m_value, detachstate); }

        int setschedpolicy(int policy)
        { return ::pthread_attr_setschedpolicy(&m_value, policy); }

        int getschedpolicy(int *policy) const
        { return ::pthread_attr_getschedpolicy(&m_value, policy); }

        int setschedparam(const struct sched_param *param)
        { return ::pthread_attr_setschedparam(&m_value, param); }
        
        int getschedparam(struct sched_param *param) const
        { return ::pthread_attr_getschedparam(&m_value, param); }

        int setinheritsched(int inherit)
        { return ::pthread_attr_setinheritsched(&m_value, inherit); }

        int getinheritsched(int *inherit) const
        { return ::pthread_attr_getinheritsched(&m_value, inherit); }

        int setscope(int scope)
        { return ::pthread_attr_setscope(&m_value, scope); }

        int getscope(int *scope) const
        { return ::pthread_attr_getscope(&m_value, scope); }

        int setguardsize(size_t guardsize)
        { return ::pthread_attr_setguardsize(&m_value, guardsize); }

        int getguardsize(size_t *guardsize) const
        { return ::pthread_attr_getguardsize(&m_value, guardsize); }

        /* stack */

        int setstack(void *stackaddr, size_t stacksize)
        { return ::pthread_attr_setstack(&m_value, stackaddr, stacksize); }

        int getstack(void **stackaddr, size_t *stacksize) const
        { return ::pthread_attr_getstack(&m_value, stackaddr, stacksize); }

        int setstacksize(size_t stacksize)
        { return ::pthread_attr_setstacksize(&m_value, stacksize); }

        int getstacksize(size_t *stacksize) const
        { return ::pthread_attr_getstacksize(&m_value, stacksize); }

        pthread_attr_t *
        operator &()
        { return &m_value; }

        const pthread_attr_t *
        operator &() const
        { return &m_value; }

    private:
        pthread_attr_t  m_value;
    };

    ////////////////////////////// base_mutex  //////////////////////////////

    template <int n>
    class __base_mutex : private noncopyable
    {    
    public:
        __base_mutex()
        : m_cancelstate_old()
        {}

        ~__base_mutex() 
        {}

    private:
        int m_cancelstate_old;
        static __thread int s_lock_cnt;

    protected:

        void use_incr()
        {
            if ( !s_lock_cnt++ ) {
                ::pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&m_cancelstate_old);
            }
        }

        void use_decr()
        {
            if (!--s_lock_cnt) {
                ::pthread_setcancelstate(m_cancelstate_old,NULL);
            }
        }

        static int
        use_count()
        { 
            return s_lock_cnt;
        }
    };

    template <int N>
#ifndef __INTEL_COMPILER 
    __thread 
#endif
    int __base_mutex<N>::s_lock_cnt = 0;

    typedef __base_mutex<0> base_mutex;

    ////////////////////////////// mutex //////////////////////////////

    class mutex : protected base_mutex
    {
        friend class cond;

    public:
        explicit mutex(pthread_mutexattr_t *attr = NULL) 
        : m_pm()
        {
            if (int err =::pthread_mutex_init(&m_pm,attr)) {
                throw more::system_error("pthread_mutex_init",err);
            }
        }

        explicit mutex(int type) 
        : m_pm()
        {        
            /* type can be either: PTHREAD_MUTEX_DEFAULT, PTHREAD_MUTEX_NORMAL, 
                               PTHREAD_MUTEX_ERRORCHECK or PTHREAD_MUTEX_RECURSIVE */

            assert ( type == PTHREAD_MUTEX_DEFAULT ||
                     type == PTHREAD_MUTEX_NORMAL  ||
                     type == PTHREAD_MUTEX_ERRORCHECK ||
                     type == PTHREAD_MUTEX_RECURSIVE);

            pthread_mutexattr_t attr; 

            if (int err = ::pthread_mutexattr_init(&attr)) {
                throw more::system_error("pthread_mutexattr_init",err);
            }
            if (int err = ::pthread_mutexattr_settype (&attr, type)) {
                throw more::system_error("pthread_mutexattr_settype", err);
            }
            if (int err = ::pthread_mutex_init(&m_pm,&attr)) {
                throw more::system_error("pthread_mutex_init", err);
            }
            if (int err = ::pthread_mutexattr_destroy (&attr)) {
                throw more::system_error("pthread_mutexattr_destroy", err);
            }
        }

        ~mutex()
        {
            if (int err = ::pthread_mutex_destroy(&m_pm)) { 
                if (err != EBUSY)
                    std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_destroy: " << 
                    more::strerror(err) << std::endl;  
            }
        }

        bool lock()
        {
            this->use_incr(); 
            if (int err = ::pthread_mutex_lock(&m_pm)) { 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_lock: " << 
                    more::strerror(err) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool unlock()
        { 
            if ( int err = ::pthread_mutex_unlock(&m_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_unlock: " << 
                    more::strerror(err) << std::endl;
                return false;
            }
            this->use_decr();
            return true; 
        }

    private:
        pthread_mutex_t m_pm;
    };

    ////////////////////////////// reader/writer mutex  //////////////////////////////

    class rw_mutex : protected base_mutex 
    {
    public:
        explicit rw_mutex(pthread_rwlockattr_t *attr = NULL) 
        : m_pm()
        { 
            if (int err = ::pthread_rwlock_init(&m_pm, attr)) { 
                throw more::system_error("pthread_rwlock_init",err);
            }
        }

        ~rw_mutex() 
        { 
            if (int err = ::pthread_rwlock_destroy(&m_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rw_mutex_destroy: " << 
                        more::strerror(err) << std::endl; 
            }
        }

        bool rdlock()
        {
            this->use_incr(); 
            if (int err = ::pthread_rwlock_rdlock(&m_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rdlock: " << 
                        more::strerror(err) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool wrlock()
        { 
            this->use_incr(); 
            if (int err = ::pthread_rwlock_wrlock(&m_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wrlock: " << 
                            more::strerror(err) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool unlock() 
        { 
            if (int err = ::pthread_rwlock_unlock(&m_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wr_ulock: " << 
                            more::strerror(err) << std::endl;
                return false;
            }
            this->use_decr();
            return true; 
        }

    private:
        pthread_rwlock_t m_pm;
    };

    struct read_mutex  : public rw_mutex {};
    struct write_mutex : public rw_mutex {};

    ////////////////////////////// scoped_lock  //////////////////////////////

    template <class M>
    class scoped_lock : private noncopyable 
    {
        friend class cond;

    public:

        typedef M mutex_type;

        template <typename T>
        scoped_lock(T &m) 
        : m_mutex( static_cast<mutex_type &>(m) )
        {   
            static_assert( (std::tr1::is_same<T, M>::value) || (std::tr1::is_base_of<T, M>::value), lock_type_unknown );

            if (!this->lock()) { 
                throw std::runtime_error("scoped_lock<>");
            }
        }

        ~scoped_lock()
        {
            if (!this->unlock())
                std::clog << __PRETTY_FUNCTION__  << std::endl;  
        }

    private:

        bool lock()
        { 
            return this->lock( type2type<M>() ); 
        }

        bool unlock()
        { 
            return m_mutex.unlock(); 
        }

        bool lock( type2type<mutex> )
        { 
            return m_mutex.lock(); 
        }

        bool lock( type2type<read_mutex> )
        { 
            return m_mutex.rdlock(); 
        }

        bool lock( type2type<write_mutex> )
        { 
            return m_mutex.wrlock(); 
        }

        mutex_type &
        get_mutex()
        { return m_mutex; }

        mutex_type & m_mutex;
    };

    ////////////////////////////// condition  //////////////////////////////

    class cond : private noncopyable
    { 
    public:
        cond(pthread_condattr_t *attr = NULL)
        : m_cond()
        {
            if (int err = ::pthread_cond_init(&m_cond, attr)) {
                throw more::system_error("pthread_cond_init",err);
            }
        }

        void signal()
        { ::pthread_cond_signal(&m_cond); }

        void broadcast()
        {
            ::pthread_cond_broadcast(&m_cond); 
        }

        template <typename M>
        int wait(scoped_lock<M> &sl) 
        {
            sl.get_mutex().use_decr();
            if (int err = ::pthread_cond_wait(&m_cond, & sl.get_mutex().m_pm)) {
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_wait: " << 
                    more::strerror(err) << std::endl;
            }
            sl.get_mutex().use_incr();
            return 0; 
        }

        template <typename M>
        int timedwait(scoped_lock<M> &sl, const struct timespec *abstime) 
        {
            sl.get_mutex().use_decr();
            if (int err = ::pthread_cond_timedwait(&m_cond, &sl.get_mutex().m_pm, abstime)){
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_timedwait: " << 
                    more::strerror(err) << std::endl;
            }
            sl.get_mutex().use_incr();
            return 0;
        }

        ~cond()
        { 
            if (int err = ::pthread_cond_destroy(&m_cond)) {
                if (err != EBUSY)
                    std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_destroy: " << 
                        more::strerror(err) << std::endl;
            }
        }

    private:
        pthread_cond_t  m_cond;

    };

    ////////////////////////////// barrier  //////////////////////////////

    class barrier : private noncopyable 
    {

    public:
        barrier(int count, const pthread_barrierattr_t * attr = NULL)
        : m_barrier()
        {
            this->init(count,attr);
        }

        ~barrier()
        {
            if (int err=pthread_barrier_destroy(&m_barrier)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_barrier_destroy: " << 
                    more::strerror(err) << std::endl;
            }
        }

        void init(int count, const pthread_barrierattr_t * attr = NULL)
        {
            if (int err = pthread_barrier_init(&m_barrier, attr, count)) 
                throw more::system_error("pthread_barrier_init",err);
        }

        void wait() 
        {
            pthread_barrier_wait(&m_barrier);        
        }
        
    private:
        pthread_barrier_t m_barrier;
    };

    ////////////////////////////// thread  //////////////////////////////

    template <typename T>
    class concrete_thread : public T
    {
        void concrete_thread_impl()
        {}

    public:
        concrete_thread()
        {}

        template <typename T1>
        concrete_thread(const T1 &t1)
        : T(t1)
        {}

        template <typename T1, typename T2>
        concrete_thread(const T1 &t1, const T2 &t2)
        : T(t1,t2)
        {}

        template <typename T1, typename T2, typename T3>
        concrete_thread(const T1 &t1, const T2 &t2, const T3 &t3)
        : T(t1,t2,t3)
        {}

        template <typename T1, typename T2, typename T3, typename T4>
        concrete_thread(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4)
        : T(t1,t2,t3,t4)
        {}

        template <typename T1, typename T2, typename T3, typename T4, typename T5>
        concrete_thread(const T1 &t1, const T2 &t2, const T3 &t3, const T4 &t4, const T5 &t5)
        : T(t1,t2,t3,t4,t5)
        {}

        ~concrete_thread()
        {
            this->cancel();
        }
    };

    class thread : private noncopyable
    {
        friend class thread_group;

        struct __global : private noncopyable
        {
            mutex               term_mutex;
            mutex               term_cond_mutex;
            cond                term_cond;
            barrier             term_barrier;

            int                 thread_cnt;
            thread *            thread_p;

            static __global &
            instance()
            {
                static __global one;
                return one;
            }

        private:
            __global()            
            : term_mutex(),
              term_cond_mutex(),
              term_cond(),
              term_barrier(1),
              thread_cnt(0),
              thread_p(0)
            {}

            ~__global()
            {}
        };

        pthread_t               m_thread;
        shared_ptr<thread_attr> m_attr;

        enum { 
               thread_not_started,
               thread_running,
               thread_terminating,
               thread_terminated,
               thread_cancelled

        } volatile m_run_state;

        volatile bool   m_joinable;

    public: 
        explicit thread(shared_ptr<thread_attr> a = shared_ptr<thread_attr>(new thread_attr))
        : m_thread(),
          m_attr(a),
          m_run_state(thread_not_started),
          m_joinable(false)
        {}

        virtual ~thread() 
        {
            assert(!m_thread || m_run_state != thread_running || 
                  !"posix::thread deleted while the thread_ruotine is running!" );
        }        
      
        template <bool delete_that> 
        friend void *thread_routine(void *);

        template <bool delete_that>
        static void *thread_routine(void *arg)
        {
            thread *that = reinterpret_cast<thread *>(arg);
            void *ret;

            ::pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); 

            try 
            {
                ret = that->operator()();
            }
            catch(std::exception &e)  // uncaught thread exceptions;
            {
                std::clog << __PRETTY_FUNCTION__ << ": uncaught exception: " << e.what() << 
                    ": thread terminated!" << std::endl;
            }
            catch(...)  // pthread_cancel causes the thread to throw an 
                        // unknown exception that is to be rethrown;
            {
                std::clog << __PRETTY_FUNCTION__ << 
                    ": pthread_cancel exception: thread terminated!" << std::endl;

                term_notify(that);

                that->m_run_state  = thread_cancelled;
                that->m_joinable   = false;
                that->m_thread     = static_cast<pthread_t>(0);

                if (delete_that)
                    delete that; 

                throw;
            }

            term_notify(that);

            that->m_run_state  = thread_terminated;
            that->m_joinable = false;
            that->m_thread   = static_cast<pthread_t>(0);

            if (delete_that)
                delete that;

            return ret;
        }

      
        bool start() 
        {
            if (m_run_state == thread_running)
                return false;   // already started

            if (int err = ::pthread_create(&m_thread,&(*m_attr),thread_routine<false>, this)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create: " << 
                    more::strerror(err) << std::endl;
                return false;
            }
            m_run_state = thread_running;
            m_joinable = true;
            return true;
        }

        template <typename T>
        static bool start_detached_in_heap() 
        {
            T * that = new concrete_thread<T>;
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1>
        static bool start_detached_in_heap(const P1 &p1) 
        {
            T * that = new concrete_thread<T>(p1);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2>
        static bool start_detached_in_heap(const P1 &p1, const P2 &p2) 
        {
            T * that = new concrete_thread<T>(p1,p2);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3>
        static bool start_detached_in_heap(const P1 &p1, const P2 &p2, const P3 &p3) 
        {
            T * that = new concrete_thread<T>(p1,p2,p3);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3, typename P4>
        static bool start_detached_in_heap(const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4) 
        {
            T * that = new concrete_thread<T>(p1,p2,p3,p4);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
        static bool start_detached_in_heap(const P1 &p1, const P2 &p2, const P3 &p3, const P4 &p4, const P5 &p5) 
        {
            T * that = new concrete_thread<T>(p1,p2,p3,p4,p5);
            return create_detached_in_heap(that);
        }

        // nore: restart() requires the concrete thread be implementing the restart_impl method
        //       example:
        //
        //       void restart_impl()
        //       {
        //           new (this) concreteThread;
        //       }

        bool restart()
        {
            this->cancel();
            this->restart_impl();
            return this->start();      
        }

        bool cancel()
        {
            pthread_t pt = this->get_id();

            if ( !pt || m_run_state != thread_running  || ::pthread_cancel(pt) == ESRCH ) {
                m_run_state = thread_cancelled;
                m_joinable = false;
                return false;
            }

            void *status = (void *)0; 
            for(;;) {

                // on joinable thread, pthread_join waits for the thread to be canceled
                //

                if ( ::pthread_join(pt, &status)  == ESRCH ) {
                    m_run_state = thread_cancelled;
                    m_joinable = false;
                    return true;
                }

                if (status == PTHREAD_CANCELED || m_run_state != thread_running)
                    break;

                std::clog << __PRETTY_FUNCTION__ << "(" << std::hex << pt << 
                    ") spinning while thread terminates..." << std::endl;
                usleep(200000);
            }

            m_run_state = thread_cancelled;
            m_joinable = false;
            return true;
        }

#define THREAD_METHOD_PRECONDITION(p)\
            if (!p || m_run_state != thread_running) {  \
                return ESRCH;  \
            }

        int 
        join(void **thread_return=NULL) const 
        {
            pthread_t p = this->get_id();
            THREAD_METHOD_PRECONDITION(p);
            return ::pthread_join(p, thread_return); 
        }

        int 
        detach()  
        { 
            pthread_t p = this->get_id();
            THREAD_METHOD_PRECONDITION(p);
            return ::pthread_detach(p); 
        }

        int 
        setschedparam(int policy, const struct sched_param *param)  
        { 
            pthread_t p = this->get_id();
            THREAD_METHOD_PRECONDITION(p);
            return ::pthread_setschedparam(p, policy, param); 
        }

        int 
        getschedparam(int *policy, struct sched_param *param) const 
        { 
            pthread_t p = this->get_id();
            THREAD_METHOD_PRECONDITION(p);
            return ::pthread_getschedparam(p, policy, param); 
        }

        int
        setschedprio(int prio)
        { 
            pthread_t p = this->get_id();
            THREAD_METHOD_PRECONDITION(p);
            return ::pthread_setschedprio(p,prio); 
        }

        int 
        kill(int signo)
        { 
            pthread_t p = this->get_id();
            THREAD_METHOD_PRECONDITION(p);
            return ::pthread_kill(p, signo); 
        }

        pthread_t 
        get_id() const 
        { return m_thread; }

        bool 
        is_running() const
        { return m_run_state == thread_running; }

        bool joinable() const
        { return m_joinable; }

    private:

        ///////////////////////////////////////////
        // thread main function: to be implemented

        virtual void *operator()() = 0;        
 
        virtual void concrete_thread_impl() = 0;  // ensure the concrete_thread<> instantiation  

        virtual void restart_impl()
        {
            throw std::runtime_error("restart not implemented in this thread");
        }

        static bool create_detached_in_heap(thread *that)
        {
            that->m_attr->setdetachstate(PTHREAD_CREATE_DETACHED);

            if (int err = ::pthread_create(&that->m_thread, &(*that->m_attr), thread_routine<true>, that )) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create: " << 
                    more::strerror(err) << std::endl;
                return false;
            }
            that->m_run_state = thread_running;
            that->m_joinable = false;
            return true;
        }

        static void term_notify(thread *that)
        {
            scoped_lock<mutex> _LM_ (__global::instance().term_mutex);
            {
                scoped_lock<mutex> _CM_ (__global::instance().term_cond_mutex);

                if (__global::instance().thread_cnt == 0) {
                    return;
                }

                // initialize the barrier
                __global::instance().term_barrier.init(__global::instance().thread_cnt+1 );

                // reset the counter...
                __global::instance().thread_cnt = 0;

                // notify the thread terminating...
                __global::instance().thread_p = that;

                // broadcast the thread joining groups
                __global::instance().term_cond.broadcast();

                // set the thread in terminating state..
                that->m_run_state = thread::thread_terminating;            
            }
            
            // wait on barrier...
            __global::instance().term_barrier.wait();
            __global::instance().thread_p =  NULL;

           // wait for other groups to join...
           // 
           // since it's not possible to predict the actual number of joining thread_groups 
           // a viable solution is to wait (a given period of time) to allow a number of them 
           // to join() for the next termination -- Nicola 09/08/14

            usleep(200000);

        }

    };

    class thread_group 
    {
        public:
            typedef std::set<thread *>::iterator                iterator;
            typedef std::set<thread *>::const_iterator          const_iterator;
            typedef std::set<thread *>::reverse_iterator        reverse_iterator;
            typedef std::set<thread *>::const_reverse_iterator  const_reverse_iterator;
            
            thread_group()
            : m_group()
            {}

            ~thread_group()
            {}

            iterator
            begin()
            { return m_group.begin(); }

            const_iterator
            begin() const
            { return m_group.begin(); }

            iterator
            end()
            { return m_group.end(); }

            const_iterator
            end() const
            { return m_group.end(); }

            reverse_iterator
            rbegin()
            { return m_group.rbegin(); }

            const_reverse_iterator
            rbegin() const
            { return m_group.rbegin(); }

            reverse_iterator
            rend()
            { return m_group.rend(); }

            const_reverse_iterator
            rend() const
            { return m_group.rend(); }

            int running() const
            {
                return count_if(m_group.begin(), m_group.end(), mem_fn(&thread::is_running));
            }

            bool add(thread *value)
            {
                return m_group.insert(value).second;    
            }

            void remove(thread *value)
            {
                m_group.erase(value);
            }

            void start_all()
            {
                for_each(m_group.begin(), m_group.end(), mem_fn(&thread::start));
            }

            void detach_all()
            {
                for_each(m_group.begin(), m_group.end(), mem_fn(&thread::detach));
            }

            void join_all()
            {
                for_each(m_group.begin(), m_group.end(), 
                         bind( mem_fn(&thread::join),_1, static_cast<void **>(NULL) ));
            }

            template <typename T>
            struct scoped_wait 
            {
                scoped_wait(T &elem)
                : m_elem(elem),
                  m_enabled(true)
                {}

                ~scoped_wait()
                {
                    if (m_enabled)
                        m_elem.wait();
                }

                void set(bool value)
                {
                    m_enabled = value;
                }

            private:
                T & m_elem;
                bool m_enabled;
            };

            template <typename T>
            void join_any(T cw)
            {
                for(;;)                   
                {
                    {
                        scoped_wait<barrier> _B_ (thread::__global::instance().term_barrier);
                        scoped_lock<mutex>   _L_ (thread::__global::instance().term_cond_mutex);

                        if (!this->running()) { 
                            _B_.set(false); // disable wait on barrier
                            return;
                        }

                        thread::__global::instance().thread_cnt++;

                        // wait on condition...
                        thread::__global::instance().term_cond.wait(_L_);

                        if ( m_group.find(
                                const_cast<thread *>(thread::__global::instance().thread_p)
                                          ) != m_group.end()) 
                        {
                            // invoke the functor
                            if (!cw(thread::__global::instance().thread_p) )
                            {
                               return; // exit... 
                            }

                        } 

                    } // unlock & wait on barrier... 
                } // go for the next termination...
            }

        private:
            std::set<thread *> m_group;
    };

    namespace this_thread 
    {
        static inline pthread_t
        get_id() 
        { 
            return pthread_self(); 
        }

        static inline pthread_t
        self()
        { 
            return pthread_self(); 
        }

        static inline int
        setcancelstate(int state, int *oldstate)
        { 
            return ::pthread_setcancelstate(state,oldstate); 
        }

        static inline int
        setcanceltype(int type, int *oldtype)
        { 
            return ::pthread_setcanceltype(type,oldtype); 
        }

        static inline void 
        testcancel() 
        { 
            ::pthread_testcancel(); 
        }

        static inline int 
        sig_mask(int how, const sigset_t * __restrict s, sigset_t * __restrict os)
        { 
            return ::pthread_sigmask(how,s,os); 
        }

        static inline int
        getconcurrency() 
        { 
            return ::pthread_getconcurrency(); 
        }

        static inline int
        setconcurrency(int new_level)
        { 
            return ::pthread_setconcurrency(new_level); 
        }

        static inline int
        key_create(pthread_key_t *key, void (*destructor)(void*))
        {
            return ::pthread_key_create(key, destructor);
        }

        static inline int
        key_delete(pthread_key_t key)
        {
            return ::pthread_key_delete(key);
        }

        static inline void *
        getspecific(pthread_key_t key) 
        {
            return ::pthread_getspecific(key);
        }
       
        static inline int
        once(pthread_once_t *once_control, void (*init_routine)(void))
        {
            return ::pthread_once(once_control, init_routine);
        }

        static inline int 
        setspecific(pthread_key_t key, const void *value)
        { 
            return ::pthread_setspecific(key, value);
        }

        static inline bool
        equal(pthread_t t1, pthread_t t2 = this_thread::get_id() )
        {
            return ::pthread_equal(t1, t2);
        }

    }

    static inline bool
    operator==(const thread &t1, const thread &t2)
    {
        return ::pthread_equal(t1.get_id(), t2.get_id());
    }

} // namespace posix
} // namespace more

#undef THREAD_METHOD_PRECONDITION

#endif /* PTHREADPP_HH */
