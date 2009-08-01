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
    namespace  
    {
        template <int n>
        struct int2type {
            enum { value = n };
        };

        template <bool N> struct static_assert;
        template <>
        struct static_assert<true>
        {
            enum { value = true };
        };
    }

    using namespace std::tr1::placeholders;
    using std::tr1::shared_ptr;
    using std::tr1::mem_fn;
    using std::tr1::bind;

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

    ////////////////////////////// base_lock  //////////////////////////////

    template <int n>
    class __base_lock
    {
    public:
        enum { simple = 0, reader = 1, writer = 2 };

        static __thread int _M_lock_cnt;

        __base_lock() 
        {}

        ~__base_lock() 
        {}

    private:
        __base_lock(const __base_lock&);                    // disable copy constructor
        __base_lock& operator=(const __base_lock&);         // disable operator= 
        void* operator new(std::size_t);                    // disable heap allocation
        void* operator new[](std::size_t);                  // disable heap allocation
        void operator delete(void*);                        // disable delete operation
        void operator delete[](void*);                      // disable delete operation
        __base_lock* operator&();                           // disable address taking
    };

    template <int n>
    __thread int __base_lock<n>::_M_lock_cnt = 0;

    typedef __base_lock<0> base_lock;

    ////////////////////////////// scoped_lock  //////////////////////////////

    template <class M, int Type = base_lock::simple >
    class scoped_lock : protected base_lock 
    {
        friend class cond;

    public:
        typedef M mutex_type;

        scoped_lock(mutex_type &m) 
        : _M_cs_old(0),
          _M_mutex(m)
        {   
            static_assert<  Type == base_lock::simple || 
                            Type == base_lock::reader || 
                            Type == base_lock::writer> lock_type_unknown __attribute__((unused));

            this->use_incr();
            if (!this->lock()) {
                this->use_decr();
                throw std::runtime_error("scoped_lock");
            }
        }

        ~scoped_lock()
        {
            if (!this->unlock())
                std::clog << __PRETTY_FUNCTION__  << ": " << strerror(errno) << std::endl;  
            this->use_decr();
        }

    private:

        void use_incr()
        {  
            if ( !base_lock::_M_lock_cnt++ ) {
                // std::cout << __PRETTY_FUNCTION__ << ": set cancelstate to PTHREAD_CANCEL_DISABLE" << std::endl; 
                ::pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&_M_cs_old);
            }
        }

        void use_decr()
        {
            if (!--base_lock::_M_lock_cnt) {
                // std::cout << __PRETTY_FUNCTION__ << ": restore calcelstate" << std::endl; 
                ::pthread_setcancelstate(_M_cs_old,NULL);
            }
        }

        bool lock()
        { return this->lock(int2type<Type>()); }

        bool unlock()
        { return _M_mutex.unlock(); }

        bool lock(int2type<base_lock::simple>)
        { return _M_mutex.lock(); }

        bool lock(int2type<base_lock::reader>)
        { return _M_mutex.rdlock(); }

        bool lock(int2type<base_lock::writer>)
        { return _M_mutex.wrlock(); }

        mutex_type &
        get_mutex()
        { return _M_mutex; }

        int _M_cs_old;

        mutex_type & _M_mutex;
    };

    ////////////////////////////// mutex //////////////////////////////

    class mutex
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

        /* type can be either: PTHREAD_MUTEX_DEFAULT, PTHREAD_MUTEX_NORMAL, 
                               PTHREAD_MUTEX_ERRORCHECK or PTHREAD_MUTEX_RECURSIVE */

        explicit mutex(int type) 
        : _M_pm()
        {
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
            if (::pthread_mutex_destroy(&_M_pm) != 0) 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_destroy: " << strerror(errno) << std::endl;  
        }

        bool lock()
        { 
            if (::pthread_mutex_lock(&_M_pm) !=0) { 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_lock: " << strerror(errno) << std::endl;
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
            return true; 
        }

    private:
        pthread_mutex_t _M_pm;
    };

    ////////////////////////////// condition  //////////////////////////////

    class cond
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

        template <typename M, int Type>
        int wait(scoped_lock<M,Type> &sl) 
        {
            sl.use_decr();
            int r = ::pthread_cond_wait(&_M_cond, & sl.get_mutex()._M_pm);
            sl.use_incr();
            if (r != 0) {
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_wait: " << strerror(errno) << std::endl;
            }
            return r; 
        }

        template <typename M, int Type>
        int timedwait(scoped_lock<M, Type> &sl, const struct timespec *abstime) 
        {
            sl.use_decr();
            int r = ::pthread_cond_timedwait(&_M_cond, &sl.get_mutex()._M_pm, abstime);
            sl.use_incr();
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

        // non-copyable idiom
        cond(const cond &);
        cond & operator=(const cond &);

    };

    ////////////////////////////// reader/writer mutex  //////////////////////////////

    class rw_mutex 
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
            if (::pthread_rwlock_rdlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rdlock: " << strerror(errno) << std::endl;
                return false;
            }
            return true; 
        }

        bool wrlock()
        { 
            if (::pthread_rwlock_wrlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wrlock: " << strerror(errno) << std::endl;
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
            return true; 
        }

    private:
        pthread_rwlock_t _M_pm;
    };

    ////////////////////////////// thread  //////////////////////////////

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
            assert(!_M_thread || 
                   _M_running != thread_running || 
                  !"posix::thread deleted while the thread ruotine is running [ probably this->cancel() call missing in the thread descructor ]!" );
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

                if ( !delete_that ) {
                    if ( global::instance().jgroup_enabled )
                        term_notify(that);

                    that->_M_running  = thread_cancelled;
                    that->_M_joinable = false;
                    that->_M_thread   = static_cast<pthread_t>(0);
           
                } else {
                    delete that;
                }

                throw;
            }

            if ( !delete_that) {
                if ( global::instance().jgroup_enabled )
                    term_notify(that);

                that->_M_running  = thread_terminated;
                that->_M_joinable = false;
                that->_M_thread   = static_cast<pthread_t>(0);

            } else {
                delete that;
            }

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

        // note: to be used in conjunction with ->stop_and_delete_this(). 
        //       the thread function (operator()) is responsible to delete the object 
        //       it refers to by means of stop_and_delete_this() method.

        template <typename T>
        static bool start_detached_in_heap() 
        {
            T * that = new T;
            return create_detached_in_heap(that);
        }

        template <typename T, typename P1>
        static bool start_detached_in_heap(P1 p1) 
        {
            T * that = new T(p1);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2>
        static bool start_detached_in_heap(P1 p1, P2 p2) 
        {
            T * that = new T(p1,p2);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3>
        static bool start_detached_in_heap(P1 p1, P2 p2, P3 p3) 
        {
            T * that = new T(p1,p2,p3);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3, typename P4>
        static bool start_detached_in_heap(P1 p1, P2 p2, P3 p3, P4 p4) 
        {
            T * that = new T(p1,p2,p3,p4);
            return create_detached_in_heap(that);
        }
        template <typename T, typename P1, typename P2, typename P3, typename P4, typename P5>
        static bool start_detached_in_heap(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5) 
        {
            T * that = new T(p1,p2,p3,p4,p5);
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

#define METHOD_PRECOND(p)\
            if (!p || _M_running != thread_running) {  \
                return ESRCH;  \
            }

        int 
        join(void **thread_return=NULL) const 
        {
            pthread_t p = this->get_id();
            METHOD_PRECOND(p);
            return ::pthread_join(p, thread_return); 
        }

        int 
        detach()  
        { 
            pthread_t p = this->get_id();
            METHOD_PRECOND(p);
            return ::pthread_detach(p); 
        }

        int 
        setschedparam(int policy, const struct sched_param *param)  
        { 
            pthread_t p = this->get_id();
            METHOD_PRECOND(p);
            return ::pthread_setschedparam(p, policy, param); 
        }

        int 
        getschedparam(int *policy, struct sched_param *param) const 
        { 
            pthread_t p = this->get_id();
            METHOD_PRECOND(p);
            return ::pthread_getschedparam(p, policy, param); 
        }

        int
        setschedprio(int prio)
        { 
            pthread_t p = this->get_id();
            METHOD_PRECOND(p);
            return ::pthread_setschedprio(p,prio); 
        }

        int 
        kill(int signo)
        { 
            pthread_t p = this->get_id();
            METHOD_PRECOND(p);
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


    protected:
        // meaningful only in the thread context -- operator() 
        
        virtual void restart_impl()
        {
            assert(!"restart not implemented in this thread");
        }

        // note: to be used in conjunction with ->start_detached_in_heap(). 
        //       on threads allocated with new and running in detached state.

        void 
        stop_and_delete_this()
        {   
            _M_running = thread_terminated;
            _M_joinable = false; 
            delete this; 
        }    

        virtual void *operator()() = 0;        
        
    private:

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

#endif /* PTHREADPP_HH */
