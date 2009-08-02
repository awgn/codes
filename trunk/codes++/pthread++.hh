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

#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>
#include <err.h>

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

    class noncopyable
    {
    protected:
        noncopyable()
        {}

        ~noncopyable()
        {}

    private:
        noncopyable(const noncopyable &);
        const noncopyable & operator=(const noncopyable &);
    };

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
            if (::pthread_mutex_destroy(&_M_pm) != 0) { 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_destroy: " << strerror(errno) << std::endl;  
            }
        }

        bool lock()
        {
            this->use_incr(); 
            if (::pthread_mutex_lock(&_M_pm) !=0) { 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_lock: " << strerror(errno) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool unlock()
        { 
            if ( ::pthread_mutex_unlock(&_M_pm) != 0) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_unlock: " << strerror(errno) << std::endl;
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
            if (::pthread_rwlock_destroy(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rw_mutex_destroy: " << strerror(errno) << std::endl; 
            }
        }

        bool rdlock()
        {
            this->use_incr(); 
            if (::pthread_rwlock_rdlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rdlock: " << strerror(errno) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool wrlock()
        { 
            this->use_incr(); 
            if (::pthread_rwlock_wrlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wrlock: " << strerror(errno) << std::endl;
                this->use_decr();
                return false;
            }
            return true; 
        }

        bool unlock() 
        { 
            if (::pthread_rwlock_unlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wr_ulock: " << strerror(errno) << std::endl;
                return false;
            }
            this->use_decr();
            return true; 
        }

    private:
        pthread_rwlock_t _M_pm;
    };

    struct read_mutex : public rw_mutex 
    {
        operator rw_mutex &()
        { return *this; }
    };

    struct write_mutex : public rw_mutex 
    {
        operator rw_mutex &()
        { return *this; }
    };

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
                std::clog << __PRETTY_FUNCTION__  << ": " << strerror(errno) << std::endl;  
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
            int r = ::pthread_cond_wait(&_M_cond, & sl.get_mutex()._M_pm);
            sl.get_mutex().use_incr();
            if (r != 0) {
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_wait: " << strerror(errno) << std::endl;
            }
            return r; 
        }

        template <typename M>
        int timedwait(scoped_lock<M> &sl, const struct timespec *abstime) 
        {
            sl.get_mutex().use_decr();
            int r = ::pthread_cond_timedwait(&_M_cond, &sl.get_mutex()._M_pm, abstime);
            sl.get_mutex().use_incr();
            if (r != 0) {
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_timedwait: " << strerror(errno) << std::endl;
            }
            return r;
        }

        ~cond()
        { 
            if (::pthread_cond_destroy(&_M_cond) != 0) {
                std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_destroy: " << strerror(errno) << std::endl;
            }
        }

    private:
        pthread_cond_t  _M_cond;

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
        concrete_thread(T1 t1)
        : T(t1)
        {}

        template <typename T1, typename T2>
        concrete_thread(T1 t1, T2 t2)
        : T(t1,t2)
        {}

        template <typename T1, typename T2, typename T3>
        concrete_thread(T1 t1, T2 t2, T3 t3)
        : T(t1,t2,t3)
        {}

        template <typename T1, typename T2, typename T3, typename T4>
        concrete_thread(T1 t1, T2 t2, T3 t3, T4 t4)
        : T(t1,t2,t3,t4)
        {}

        template <typename T1, typename T2, typename T3, typename T4, typename T5>
        concrete_thread(T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
        : T(t1,t2,t3,t4,t5)
        {}

        ~concrete_thread()
        {
            this->cancel();
        }
    };


    class thread 
    {
        struct global
        {
            volatile bool         jgroup_enabled;
            sem_t                 term_sem;
            mutex                 term_mutex;
            thread * volatile     thread_id;

            static global &
            instance(int n = 0)
            {
                static global one(n);
                return one;
            }

        private:

            global(const global &);
            global & operator=(const global &);

            global(int n)
            : jgroup_enabled(false), term_sem(), term_mutex(), thread_id(NULL)
            {
                sem_init(&term_sem,0,n);
            }

            ~global()
            {
                sem_destroy(&term_sem);
            }
        };

        pthread_t               _M_thread;
        shared_ptr<thread_attr> _M_attr;

        enum { 
               thread_not_started,
               thread_running,
               thread_cancelled,
               thread_terminated

        } volatile _M_running;

        volatile bool   _M_joinable;

        thread(const thread &);               // noncopyable
        thread &operator=(const thread &);    // noncopyable

    public: 
        explicit thread(shared_ptr<thread_attr> a = shared_ptr<thread_attr>(new thread_attr))
        : _M_thread(),
          _M_attr(a),
          _M_running(thread_not_started),
          _M_joinable(false)
        {}

        virtual ~thread() 
        {
            assert(!_M_thread || _M_running != thread_running || 
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
                std::clog << __PRETTY_FUNCTION__ << ": uncaught exception: " << e.what() << ": thread terminated!" << std::endl;
            }
            catch(...)  // pthread_cancel causes the thread to throw an unknown exception that is to be rethrown;
            {
                std::clog << __PRETTY_FUNCTION__ << ": pthread_cancel exception: thread terminated!" << std::endl;

                if ( global::instance().jgroup_enabled )
                    term_notify(that);

                that->_M_running  = thread_cancelled;
                that->_M_joinable = false;
                that->_M_thread   = static_cast<pthread_t>(0);

                if (delete_that)
                    delete that; 

                throw;
            }

            if ( global::instance().jgroup_enabled )
                term_notify(that);

            that->_M_running  = thread_terminated;
            that->_M_joinable = false;
            that->_M_thread   = static_cast<pthread_t>(0);

            if (delete_that)
                delete that;

            return ret;
        }

      
        bool start() 
        {
            if (_M_running == thread_running)
                return false;   // already started

            if (::pthread_create(&_M_thread, &(*_M_attr), thread_routine<false>, this ) != 0) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create: " << strerror(errno) << std::endl;
                return false;
            }
            _M_running = thread_running;
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
        static bool start_detached_in_heap(P1 p1) 
        {
            T * that = new concrete_thread<T>(p1);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2>
        static bool start_detached_in_heap(P1 p1, P2 p2) 
        {
            T * that = new concrete_thread<T>(p1,p2);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3>
        static bool start_detached_in_heap(P1 p1, P2 p2, P3 p3) 
        {
            T * that = new concrete_thread<T>(p1,p2,p3);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3, typename P4>
        static bool start_detached_in_heap(P1 p1, P2 p2, P3 p3, P4 p4) 
        {
            T * that = new concrete_thread<T>(p1,p2,p3,p4);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
        static bool start_detached_in_heap(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) 
        {
            T * that = new concrete_thread<T>(p1,p2,p3,p4,p5);
            return create_detached_in_heap(that);
        }

        // nore: restart() requires the concrete thread to be implementing the restart_impl method
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

            if ( !pt || _M_running != thread_running  || ::pthread_cancel(pt) == ESRCH ) {
                _M_running = thread_cancelled;
                _M_joinable = false;
                return false;
            }

            void *status = (void *)0; 
            for(;;) {

                // on joinable thread, pthread_join waits for the thread to be canceled
                //

                if ( ::pthread_join(pt, &status)  == ESRCH ) {
                    _M_running = thread_cancelled;
                    _M_joinable = false;
                    return true;
                }

                if (status == PTHREAD_CANCELED || _M_running != thread_running)
                    break;

                std::clog << __PRETTY_FUNCTION__ << "(" << std::hex << pt << ") spinning while thread terminates..." << std::endl;
                usleep(200000);
            }

            _M_running = thread_cancelled;
            _M_joinable = false;
            return true;
        }

#define THREAD_METHOD_PRECONDITION(p)\
            if (!p || _M_running != thread_running) {  \
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
        { return _M_running == thread_running; }

        bool joinable() const
        { return _M_joinable; }

    private:

        ///////////////////////////////////////////
        // thread main function: to be implemented

        virtual void *operator()() = 0;        
 
        virtual void concrete_thread_impl() = 0;  // to enforce the concrete_thread<> usage class  

        virtual void restart_impl()
        {
            assert(!"restart not implemented in this thread");
        }

        static bool create_detached_in_heap(thread *that)
        {
            that->_M_attr->setdetachstate(PTHREAD_CREATE_DETACHED);

            if (::pthread_create(&that->_M_thread, &(*that->_M_attr), thread_routine<true>, that ) != 0) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create: " << strerror(errno) << std::endl;
                return false;
            }
            that->_M_running = thread_running;
            that->_M_joinable = false;
            return true;
        }

        static void term_notify(thread *that)
        {
            scoped_lock<mutex> _L_ (global::instance().term_mutex);

            for(;;) {
                
                if (!global::instance().jgroup_enabled)
                    return;

                if ( global::instance().thread_id == NULL ) {
                    
                    global::instance().thread_id = that;
                    
                    timeval now; gettimeofday(&now,NULL);
                    const timespec timeo = { now.tv_sec + 2, 0 };
                  
                    if ( sem_timedwait(&global::instance().term_sem, &timeo) == 0 )
                        return;

                    if ( errno == ETIMEDOUT ) {
                        std::clog << __PRETTY_FUNCTION__ << ": sem_timedwait() TIMEDOUT! (termination notification would be lost)" << std::endl;
                    }
                    else {
                        std::clog << __PRETTY_FUNCTION__ << ": sem_timedwait: " << strerror(errno) << std::endl;
                    }

                    global::instance().thread_id = NULL;
                    continue;   // retry now
                } 
                
                std::clog << __PRETTY_FUNCTION__ << ": global::instance().thread_id slot busy!?!?" << std::endl;
                usleep(1000);   // just to relax the CPU and retry... 
            } 
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
                for_each(_M_group.begin(), _M_group.end(), bind( mem_fn(&thread::join),_1, static_cast<void **>(NULL) ));
            }

            template <typename T>
            void join_all(T cw)
            {
                thread::global::instance().thread_id  = NULL; 
                thread::global::instance().jgroup_enabled = true;

                for(;;) {                    

                    if (this->running()==0) {
                        thread::global::instance().jgroup_enabled = false;
                        return;
                    }

                    if (thread::global::instance().thread_id) {
                        
                        if ( _M_group.find(const_cast<thread *>(thread::global::instance().thread_id)) != _M_group.end()) {

                            cw(thread::global::instance().thread_id);

                            thread::global::instance().thread_id  = NULL;
                            sem_post(&thread::global::instance().term_sem);

                        }
                        continue;
                    }
                    usleep(1000); // just to relax the cpu...
                }
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
