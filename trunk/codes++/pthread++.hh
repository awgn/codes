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
#include <algorithm>
#include <set>

#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <err.h>

namespace more { namespace posix 
{
    using namespace std::tr1::placeholders;
    using std::tr1::shared_ptr;
    using std::tr1::mem_fn;
    using std::tr1::bind;

    template <int n>
    struct int2type {
        enum { value = n };
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

    /////////// __base_lock ////////////

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
            assert(Type == base_lock::simple || Type == base_lock::reader || Type == base_lock::writer);
            this->use_incr();
            this->lock();
        }

        ~scoped_lock()
        {
            this->unlock();
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

        void lock()
        { this->lock(int2type<Type>()); }

        void unlock()
        { _M_mutex.unlock(); }

        void lock(int2type<base_lock::simple>)
        { _M_mutex.lock(); }

        void lock(int2type<base_lock::reader>)
        { _M_mutex.rdlock(); }

        void lock(int2type<base_lock::writer>)
        { _M_mutex.wrlock(); }

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
        : _M_pm(), _M_state(false)
        { 
            if (pthread_mutex_init(&_M_pm,attr) != 0) {
                std::clog << __PRETTY_FUNCTION__ << ": pthread_mutex_init error!" << std::endl; 
                return;
            }
            _M_state = true;
        }

        ~mutex()
        {
            if (pthread_mutex_destroy(&_M_pm) != 0) 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_destroy error!" << std::endl;  
        }

        bool lock()
        { 
            if (::pthread_mutex_lock(&_M_pm) !=0) { 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_lock error!" << std::endl;
                return false;
            }
            return true; 
        }

        bool unlock()
        { 
            if ( ::pthread_mutex_unlock(&_M_pm) != 0) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_unlock error!" << std::endl;
                return false;
            }
            return true; 
        }

        operator bool() const
        { return _M_state; }

    private:
        pthread_mutex_t _M_pm;
        bool _M_state;
    };

    ////////////////////////////// cond  //////////////////////////////

    class cond
    { 
    public:
        cond(pthread_condattr_t *attr = NULL)
        : _M_cond(), _M_state(false)
        {
            if (pthread_cond_init(&_M_cond, attr) != 0) {
                std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_init error!" << std::endl; 
                return;
            }
            _M_state = true;
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
            int ret = ::pthread_cond_wait(&_M_cond, & sl.get_mutex()._M_pm);
            sl.use_incr();
            return ret; 
        }

        template <typename M, int Type>
        int timedwait(scoped_lock<M, Type> &sl, const struct timespec *abstime) 
        {
            sl.use_decr();
            int r = ::pthread_cond_timedwait(&_M_cond, &sl.get_mutex()._M_pm, abstime);
            sl.use_incr();
            if ( r != 0 ) {
                 std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_timedwait error!" << std::endl;
            }
            return r;
        }

        ~cond()
        { 
            if (::pthread_cond_destroy(&_M_cond) != 0) {
                std::clog << __PRETTY_FUNCTION__ << ": pthread_cond_destroy error!" << std::endl;
            }
        }

        operator bool() const
        { return _M_state; }

    private:
        pthread_cond_t  _M_cond;
        bool            _M_state;

        // non-copyable idiom
        cond(const cond &);
        cond & operator=(const cond &);

    };

    ////////////////////////////// rw_mutex  //////////////////////////////

    class rw_mutex 
    {
    public:
        explicit rw_mutex(pthread_rwlockattr_t *attr = NULL) 
        : _M_pm(), _M_state(false)
        { 
            if ( pthread_rwlock_init(&_M_pm, attr) != 0 ) { 
                std::clog << __PRETTY_FUNCTION__  << ": pthread_mutex_init error!" << std::endl; 
                return;
            }
            _M_state = true;
        }

        ~rw_mutex() 
        { 
            if (pthread_rwlock_destroy(&_M_pm) != 0 )
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rw_mutex_destroy error!" << std::endl; 
        }

        bool rdlock()
        {
            if ( pthread_rwlock_rdlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_rdlock error!" << std::endl;
                return false;
            }
            return true; 
        }

        bool wrlock()
        { 
            if ( pthread_rwlock_wrlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wrlock error!" << std::endl;
                return false;
            }
            return true; 
        }

        bool unlock() 
        { 
            if ( pthread_rwlock_unlock(&_M_pm) != 0 ) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_wr_ulock error!" << std::endl;
                return false;
            }
            return true; 
        }

        operator bool() const
        { return _M_state; }

    private:
        pthread_rwlock_t _M_pm;
        bool _M_state;
    };

    template <int n>
    __thread int __base_lock<n>::_M_lock_cnt = 0;

    ////////////////////////////// thread  //////////////////////////////

    class thread 
    {
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
                  !"posix::thread deleted while the thread ruotine is running!" );
        }        
        
        // for join_one method...
        //

        static cond &
        terminate_cond()
        {
            static cond ret;
            return ret;
        } 
        static mutex &
        terminate_mutex()
        {
            static mutex ret;
            return ret;
        } 

        static thread * &
        terminate_thread()
        {
            static thread * ret;
            return ret;
        }

        friend void *start_routine(void *);
        static void *start_routine(void *arg)
        {
            thread *that = reinterpret_cast<thread *>(arg);
            void *ret;

            pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); 

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

                that->_M_running = thread_cancelled;
                that->_M_joinable = false;
                that->_M_thread = static_cast<pthread_t>(0);

                scoped_lock<mutex> lock(terminate_mutex());
                terminate_thread() = that;
                terminate_cond().signal();
                throw;
            }

            that->_M_running = thread_terminated;
            that->_M_joinable = false;
            that->_M_thread = static_cast<pthread_t>(0);
            
            scoped_lock<mutex> lock(terminate_mutex());
            terminate_thread() = that;
            terminate_cond().signal();
            return ret;
        }

        friend void *start_detached_routine(void *arg);
        static void *start_detached_routine(void *arg)
        {
            thread *that = reinterpret_cast<thread *>(arg);
            void *ret;

            pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL); 

            try 
            {
                ret = that->operator()();
            }
            catch(std::exception &e)  // application exception;
            {
                std::clog << __PRETTY_FUNCTION__ << ": uncaught exception: " << e.what() << ": thread terminated!" << std::endl;
                // that->_M_running = thread_terminated; 
                // that->_M_joinable = false;
                delete that;
            }
            catch(...)  // pthread_cancel causes the thread to throw an unknown exception that is to be rethrown;
            {
                std::clog << __PRETTY_FUNCTION__ << ": pthread_cancel exception: thread terminated!" << std::endl;
                // that->_M_running = thread_cancelled; 
                // that->_M_joinable = false;
                delete that;
                throw;
            }

            return ret;
        }
      

        bool start() 
        {
            if (_M_running == thread_running)
                return false;   // already started

            if (::pthread_create(&_M_thread, &(*_M_attr), start_routine, this ) != 0) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create error!" << std::endl;
                return false;
            }
            _M_running = thread_running;
            _M_joinable = true;
            return true;
        }

        // note: to be used in conjunction with ->stop_and_delete_this() 
        //       on threads allocated with new and running in detached state.
        //       the thread function (operator()) is responsible to delete the object 
        //       it refers to by means of stop_and_delete_this() method.

        bool start_detached_in_heap() 
        {
            if (_M_running == thread_running)
                return false;   // already started
            
            _M_attr->setdetachstate(PTHREAD_CREATE_DETACHED);

            if (::pthread_create(&_M_thread, &(*_M_attr), start_detached_routine, this ) != 0) {
                std::clog << __PRETTY_FUNCTION__  << ": pthread_create error!" << std::endl;
                return false;
            }
            _M_running = thread_running;
            _M_joinable = false;
            return true;
        }

        // nore: this requires the concrete thread to be implementing the restart_impl method
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
            assert(!"restart not implemented in this concrete thread");
        }

        int
        setcancelstate(int state, int *oldstate)
        { return ::pthread_setcancelstate(state,oldstate); }

        int
        setcanceltype(int type, int *oldtype)
        { return ::pthread_setcanceltype(type,oldtype); }

        void 
        testcancel() 
        { ::pthread_testcancel(); }

        pthread_t self() const
        { return ::pthread_self(); }

        int 
        psigmask(int how, const sigset_t * __restrict s, sigset_t * __restrict os)
        { return ::pthread_sigmask(how,s,os); }

        int
        getconcurrency() const
        { return ::pthread_getconcurrency(); }

        int
        setconcurrency(int new_level)
        { return ::pthread_setconcurrency(new_level); }

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
    };

    class thread_group 
    {
        public:
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

            thread *join_one()
            {
                scoped_lock<mutex> lock(thread::terminate_mutex());
                for(;;) {
                    thread::terminate_cond().wait(lock);
                    if ( _M_group.find(thread::terminate_thread()) != _M_group.end())
                        return thread::terminate_thread();
                } 
                return NULL;           
            }

        private:
            std::set<thread *> _M_group;
    };

} // namespace posix
} // namespace more

#endif /* PTHREADPP_HH */
