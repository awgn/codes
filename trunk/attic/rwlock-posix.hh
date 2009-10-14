/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef RWLOCK_POSIX_H
#define RWLOCK_POSIX_H

#include <string>
#include <stdexcept>

#include <pthread.h>
#include <errno.h>

// this is a RAII implementation of the posix thread rwlock and rwmutex..
//

namespace posix 
{
    class RWmutex 
    {
    public:
        enum Type {
            reader,
            writer
        };

        template <RWmutex::Type RW> friend class RWlock;

        explicit RWmutex(pthread_rwlockattr_t *attr = NULL) :
        lock()
        {
            if ( pthread_rwlock_init(&lock, attr) != 0 )
                throw std::runtime_error(std::string("pthread_rwlock_init: ").append(strerror(errno)));
        }
        ~RWmutex() {
            if (pthread_rwlock_destroy(&lock) != 0 )
                std::clog << "pthread_rwlock_destroy: " << strerror(errno) << std::endl;
        }

    private:
        pthread_rwlock_t lock;

        int rdlock() {
            int r;
            if ( (r=pthread_rwlock_rdlock(&lock)) != 0 ) 
                throw std::runtime_error(std::string("pthread_rwlock_rdlock: ").append(strerror(errno)));
            return r;
        }
        int wrlock() {
            int r;
            if ( (r=pthread_rwlock_wrlock(&lock)) != 0 )
                throw std::runtime_error(std::string("pthread_rwlock_wrlock: ").append(strerror(errno)));
            return r;
        }
        int unlock() {
            int r;
            if ( (r=pthread_rwlock_unlock(&lock)) != 0 )
                throw std::runtime_error(std::string("pthread_rwlock_unlock: ").append(strerror(errno)));
            return r;
        }
    };

    // ... according to http://www.informit.com/articles/article.aspx?p=25298&rl=1
    // scoped_lock version of RWlock...

    template <RWmutex::Type RW> 
    class RWlock 
    {
        RWmutex &mtx;

        RWlock();                               // disable default constructor
        RWlock(const RWlock&);                  // disable copy constructor
        RWlock& operator=(const RWlock&);       // disable operator= 
        void* operator new(std::size_t);        // disable heap allocation
        void* operator new[](std::size_t);      // disable heap allocation
        void operator delete(void*);            // disable delete operation
        void operator delete[](void*);          // disable delete operation
        RWlock* operator&();                    // disable address taking

    public:
        RWlock(RWmutex &x) : mtx(x) {
            mtx.rdlock();
        }
        ~RWlock() {
            mtx.unlock();
        }
    };


    template <>
    class RWlock<RWmutex::writer> 
    {
        RWmutex &mtx;

        RWlock();                               // disable default constructor
        RWlock(const RWlock&);                  // disable copy constructor
        RWlock& operator=(const RWlock&);       // disable operator= 
        void* operator new(std::size_t);        // disable heap allocation
        void* operator new[](std::size_t);      // disable heap allocation
        void operator delete(void*);            // disable delete operation
        void operator delete[](void*);          // disable delete operation
        RWlock* operator&();                    // disable address taking

    public:
        RWlock(RWmutex &x) : mtx(x) {
            mtx.wrlock();
        }
        ~RWlock() {
            mtx.unlock();
        }
    };

} // namespace posix 

#endif /* RWLOCK_POSIX_H */

