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

#include <tr1/memory>
#include <tr1/functional>
#include <stdexcept>
#include <iostream>
#include <cassert>
#include <cstring>
#include <algorithm>
#include <set>

#include <noncopyable.hh>

#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

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

        template <bool N> struct static_assert;
        template <>
        struct static_assert<true>
        {
            enum { value = true };
        };
    }

    class thread_attr
    {
    public:
        thread_attr()
        : _M_value()
        { ::pthread_attr_init(&_M_value); }

        ~thread_attr()
        { /* ::pthread_attr_destroy(&_M_value); */ }

        int setdetachstate(int detachstate)
        { return ::pthread_attr_setdetachstate(&_M_value, detachstate); }

        int getdetachstate(int *detachstate) const
        { return ::pthread_attr_getdetachstate(&_M_value, detachstate); }

        int setschedpolicy(int policy)
        { return ::pthread_attr_setschedpolicy(&_M_value, policy); }

        int getschedpolicy(int *policy) const
        { return ::pthread_attr_getschedpolicy(&_M_value, policy); }

        int setschedparam(const struct sched_param *param)
        { return ::pthread_attr_setschedparam(&_M_value, param); }
        
        int getschedparam(struct sched_param *param) const
        { return ::pthread_attr_getschedparam(&_M_value, param); }

        int setinheritsched(int inherit)
        { return ::pthread_attr_setinheritsched(&_M_value, inherit); }

        int getinheritsched(int *inherit) const
        { return ::pthread_attr_getinheritsched(&_M_value, inherit); }

        int setscope(int scope)
        { return ::pthread_attr_setscope(&_M_value, scope); }

        int getscope(int *scope) const
        { return ::pthread_attr_getscope(&_M_value, scope); }

        int setguardsize(size_t guardsize)
        { return ::pthread_attr_setguardsize(&_M_value, guardsize); }

        int getguardsize(size_t *guardsize) const
        { return ::pthread_attr_getguardsize(&_M_value, guardsize); }

        /* stack */

        int setstack(void *stackaddr, size_t stacksize)
        { return ::pthread_attr_setstack(&_M_value, stackaddr, stacksize); }

        int getstack(void **stackaddr, size_t *stacksize) const
        { return ::pthread_attr_getstack(&_M_value, stackaddr, stacksize); }

        int setstacksize(size_t stacksize)
        { return ::pthread_attr_setstacksize(&_M_value, stacksize); }

        int getstacksize(size_t *stacksize) const
        { return ::pthread_attr_getstacksize(&_M_value, stacksize); }

        pthread_attr_t *
        operator &()
        { return &_M_value; }

        const pthread_attr_t *
        operator &() const
        { return &_M_value; }

    private:
        pthread_attr_t  _M_value;
    };

    ////////////////////////////// base_mutex  //////////////////////////////

    template <int n>
    class __base_mutex : private noncopyable
    {    
    public:

        __base_mutex()
        : _M_cancelstate_old()
        {}

        ~__base_mutex() 
        {}

    private:
        int _M_cancelstate_old;

        static __thread int _S_lock_cnt;

    protected:

        void use_incr()
        {
            if ( !_S_lock_cnt++ ) {
                ::pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&_M_cancelstate_old);
            }
        }

        void use_decr()
        {
            if (!--_S_lock_cnt) {
                ::pthread_setcancelstate(_M_cancelstate_old,NULL);
            }
        }

        static int
        use_count()
        { 
            return _S_lock_cnt;
        }
    };

    template <int n>
    __thread int __base_mutex<n>::_S_lock_cnt = 0;

    typedef __base_mutex<0> base_mutex;

    ////////////////////////////// mutex //////////////////////////////

    class mutex : protected base_mutex
    {
        friend class cond;

    public:
        explicit mutex(pthread_mutexattr_t *attr = NULL) 
        : _M_pm()
        { 
            if (::pthread_mutex_init(&_M_pm,attr) != 0) {
                throw std::runtime_error("pthread_mutex_init");
            }
        }

        explicit mutex(int type) 
        : _M_pm()
        {        
            /* type can be either: PTHREAD_MUTEX_DEFAULT, PTHREAD_MUTEX_NORMAL, 
                               PTHREAD_MUTEX_ERRORCHECK or PTHREAD_MUTEX_RECURSIVE */

            assert ( type == PTHREAD_MUTEX_DEFAULT ||
                     type == PTHREAD_MUTEX_NORMAL  ||
                     type == PTHREAD_MUTEX_ERRORCHECK ||
                     type == PTHREAD_MUTEX_RECURSIVE);

            pthread_mutexattr_t attr; 

            if (::pthread_mutexattr_init(&attr) != 0 ) {
                throw std::runtime_error("pthread_mutexattr_init");
            }
            if (::pthread_mutexattr_settype (&attr, type) != 0) {
                throw std::runtime_error("pthread_mutexattr_settype");
            }
            if (::pthread_mutex_init(&_M_pm,&attr) != 0) {
                throw std::runtime_error("pthread_mutex_init");
            }
            if (::pthread_mutexattr_destroy (&attr) != 0) {
                throw std::runtime_error("pthread_mutexattr_destroy");
            }
        }

        ~mutex()
        {
            if (int err = ::pthread_mutex_destroy(&_M_pm)) { 
                if (err != EBUSY)
                    std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_destroy: " << 
                    strerror(err) << std::endl;  
            }
        }

        bool lock()
        {
            this->use_incr(); 
            if (int err = ::pthread_mutex_lock(&_M_pm)) { 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_lock: " << 
                    strerror(err) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool unlock()
        { 
            if ( int err = ::pthread_mutex_unlock(&_M_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_unlock: " << 
                    strerror(err) << std::endl;
                return false;
            }
            this->use_decr();
            return true; 
        }

    private:
        pthread_mutex_t _M_pm;
    };

    ////////////////////////////// reader/writer mutex  //////////////////////////////

    class rw_mutex : protected base_mutex 
    {
    public:
        explicit rw_mutex(pthread_rwlockattr_t *attr = NULL) 
        : _M_pm()
        { 
            if (::pthread_rwlock_init(&_M_pm, attr) != 0 ) { 
                throw std::runtime_error("pthread_rwlock_init");
            }
        }

        ~rw_mutex() 
        { 
            if (int err = ::pthread_rwlock_destroy(&_M_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rw_mutex_destroy: " << 
                        strerror(err) << std::endl; 
            }
        }

        bool rdlock()
        {
            this->use_incr(); 
            if (int err = ::pthread_rwlock_rdlock(&_M_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rdlock: " << 
                        strerror(err) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool wrlock()
        { 
            this->use_incr(); 
            if (int err = ::pthread_rwlock_wrlock(&_M_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wrlock: " << 
                            strerror(err) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool unlock() 
        { 
            if (int err = ::pthread_rwlock_unlock(&_M_pm)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wr_ulock: " << 
                            strerror(err) << std::endl;
                return false;
            }
            this->use_decr();
            return true; 
        }

    private:
        pthread_rwlock_t _M_pm;
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
        : _M_mutex( static_cast<mutex_type &>(m) )
        {   
            static_assert<  std::tr1::is_same<T, M>::value || 
                            std::tr1::is_base_of<T, M>::value > 
                            lock_type_unknown __attribute__((unused));

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
            return _M_mutex.unlock(); 
        }

        bool lock( type2type<mutex> )
        { 
            return _M_mutex.lock(); 
        }

        bool lock( type2type<read_mutex> )
        { 
            return _M_mutex.rdlock(); 
        }

        bool lock( type2type<write_mutex> )
        { 
            return _M_mutex.wrlock(); 
        }

        mutex_type &
        get_mutex()
        { return _M_mutex; }

        mutex_type & _M_mutex;
    };

    ////////////////////////////// condition  //////////////////////////////

    class cond : private noncopyable
    { 
    public:
        cond(pthread_condattr_t *attr = NULL)
        : _M_cond()
        {
            if (::pthread_cond_init(&_M_cond, attr) != 0) {
                throw std::runtime_error("pthread_cond_init");
            }
        }

        void signal()
        { ::pthread_cond_signal(&_M_cond); }

        void broadcast()
        {
            ::pthread_cond_broadcast(&_M_cond); 
        }

        template <typename M>
        int wait(scoped_lock<M> &sl) 
        {
            sl.get_mutex().use_decr();
            if (int err = ::pthread_cond_wait(&_M_cond, & sl.get_mutex()._M_pm)) {
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_wait: " << 
                    strerror(err) << std::endl;
            }
            sl.get_mutex().use_incr();
            return 0; 
        }

        template <typename M>
        int timedwait(scoped_lock<M> &sl, const struct timespec *abstime) 
        {
            sl.get_mutex().use_decr();
            if (int err = ::pthread_cond_timedwait(&_M_cond, &sl.get_mutex()._M_pm, abstime)){
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_timedwait: " << 
                    strerror(err) << std::endl;
            }
            sl.get_mutex().use_incr();
            return 0;
        }

        ~cond()
        { 
            if (int err = ::pthread_cond_destroy(&_M_cond)) {
                if (err != EBUSY)
                    std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_destroy: " << 
                        strerror(err) << std::endl;
            }
        }

    private:
        pthread_cond_t  _M_cond;

    };

    ////////////////////////////// barrier  //////////////////////////////

    class barrier : private noncopyable 
    {

    public:
        barrier(int count, const pthread_barrierattr_t * attr = NULL)
        : _M_barrier()
        {
            if (this->init(count,attr)) {
                throw std::runtime_error("pthread_barrier_init");
            }
        }

        ~barrier()
        {
            if (int err=pthread_barrier_destroy(&_M_barrier)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_barrier_destroy: " << 
                    strerror(err) << std::endl;
            }
        }

        int init(int count, const pthread_barrierattr_t * attr = NULL)
        {
            if (int err=pthread_barrier_init(&_M_barrier, attr, count)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_barrier_init: " << 
                    strerror(err) << std::endl;
                return err;
            }
            return 0; 
        }

        void wait() 
        {
            pthread_barrier_wait(&_M_barrier);        
        }
        
    private:
        pthread_barrier_t _M_barrier;
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

        pthread_t               _M_thread;
        shared_ptr<thread_attr> _M_attr;

        enum { 
               thread_not_started,
               thread_running,
               thread_terminating,
               thread_terminated,
               thread_cancelled

        } volatile _M_run_state;

        volatile bool   _M_joinable;

    public: 
        explicit thread(shared_ptr<thread_attr> a = shared_ptr<thread_attr>(new thread_attr))
        : _M_thread(),
          _M_attr(a),
          _M_run_state(thread_not_started),
          _M_joinable(false)
        {}

        virtual ~thread() 
        {
            assert(!_M_thread || _M_run_state != thread_running || 
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

                that->_M_run_state  = thread_cancelled;
                that->_M_joinable   = false;
                that->_M_thread     = static_cast<pthread_t>(0);

                if (delete_that)
                    delete that; 

                throw;
            }

            term_notify(that);

            that->_M_run_state  = thread_terminated;
            that->_M_joinable = false;
            that->_M_thread   = static_cast<pthread_t>(0);

            if (delete_that)
                delete that;

            return ret;
        }

      
        bool start() 
        {
            if (_M_run_state == thread_running)
                return false;   // already started

            if (int err = ::pthread_create(&_M_thread,&(*_M_attr),thread_routine<false>, this)) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create: " << 
                    strerror(err) << std::endl;
                return false;
            }
            _M_run_state = thread_running;
            _M_joinable = true;
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

            if ( !pt || _M_run_state != thread_running  || ::pthread_cancel(pt) == ESRCH ) {
                _M_run_state = thread_cancelled;
                _M_joinable = false;
                return false;
            }

            void *status = (void *)0; 
            for(;;) {

                // on joinable thread, pthread_join waits for the thread to be canceled
                //

                if ( ::pthread_join(pt, &status)  == ESRCH ) {
                    _M_run_state = thread_cancelled;
                    _M_joinable = false;
                    return true;
                }

                if (status == PTHREAD_CANCELED || _M_run_state != thread_running)
                    break;

                std::clog << __PRETTY_FUNCTION__ << "(" << std::hex << pt << 
                    ") spinning while thread terminates..." << std::endl;
                usleep(200000);
            }

            _M_run_state = thread_cancelled;
            _M_joinable = false;
            return true;
        }

#define THREAD_METHOD_PRECONDITION(p)\
            if (!p || _M_run_state != thread_running) {  \
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
        { return _M_thread; }

        bool 
        is_running() const
        { return _M_run_state == thread_running; }

        bool joinable() const
        { return _M_joinable; }

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
            that->_M_attr->setdetachstate(PTHREAD_CREATE_DETACHED);

            if (int err = ::pthread_create(&that->_M_thread, &(*that->_M_attr), thread_routine<true>, that )) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create: " << 
                    strerror(err) << std::endl;
                return false;
            }
            that->_M_run_state = thread_running;
            that->_M_joinable = false;
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
                that->_M_run_state = thread::thread_terminating;            
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
            : _M_group()
            {}

            ~thread_group()
            {}

            iterator
            begin()
            { return _M_group.begin(); }

            const_iterator
            begin() const
            { return _M_group.begin(); }

            iterator
            end()
            { return _M_group.end(); }

            const_iterator
            end() const
            { return _M_group.end(); }

            reverse_iterator
            rbegin()
            { return _M_group.rbegin(); }

            const_reverse_iterator
            rbegin() const
            { return _M_group.rbegin(); }

            reverse_iterator
            rend()
            { return _M_group.rend(); }

            const_reverse_iterator
            rend() const
            { return _M_group.rend(); }

            int running() const
            {
                return count_if(_M_group.begin(), _M_group.end(), mem_fn(&thread::is_running));
            }

            bool add(thread *value)
            {
                return _M_group.insert(value).second;    
            }

            void remove(thread *value)
            {
                _M_group.erase(value);
            }

            void start_all()
            {
                for_each(_M_group.begin(), _M_group.end(), mem_fn(&thread::start));
            }

            void detach_all()
            {
                for_each(_M_group.begin(), _M_group.end(), mem_fn(&thread::detach));
            }

            void join_all()
            {
                for_each(_M_group.begin(), _M_group.end(), 
                         bind( mem_fn(&thread::join),_1, static_cast<void **>(NULL) ));
            }

            template <typename T>
            struct scoped_wait 
            {
                scoped_wait(T &elem)
                : _M_elem(elem),
                  _M_enabled(true)
                {}

                ~scoped_wait()
                {
                    if (_M_enabled)
                        _M_elem.wait();
                }

                void set(bool value)
                {
                    _M_enabled = value;
                }

            private:
                T & _M_elem;
                bool _M_enabled;
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

                        if ( _M_group.find(
                                const_cast<thread *>(thread::__global::instance().thread_p)
                                          ) != _M_group.end()) 
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
            std::set<thread *> _M_group;
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
