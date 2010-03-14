/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef __SEMAPHORE_HH__
#define __SEMAPHORE_HH__

#include <semaphore.h>      
#include <error.hh>         // more!
#include <noncopyable.hh>   // more!

#include <stdexcept>
#include <string>
#include <cstring>
#include <iostream>

namespace more { namespace posix {

        class semaphore : public noncopyable
        {
            public:
                semaphore(unsigned int value, int pshared = 0)            
                : _M_sem()
                {
                    if ( this->init(value, pshared) < 0 )
                        throw more::syscall_error("sem_init", errno);
                }

                ~semaphore()
                {
                    if( sem_destroy(&_M_sem) < 0)
                       std::clog << "sem_destroy: " << pretty_strerror(errno) << std::endl; 
                }

                int init(unsigned int value, int pshared)          
                {      
                    return sem_init(&_M_sem, pshared, value);
                }
                   
                int wait() const
                {
                    if ( sem_wait(&_M_sem) < 0 ) {
                        std::clog << "sem_wait: " << pretty_strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int trywait() const
                {
                    if ( sem_trywait(&_M_sem) < 0 ) {
                        std::clog << "sem_wait: " << pretty_strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int timedwait(const struct timespec *abs_timeout) const
                {
                    if ( sem_timedwait(&_M_sem, abs_timeout) < 0 ) {
                        std::clog << "sem_timedwait: " << pretty_strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int post()
                {
                    if ( sem_post(&_M_sem) < 0 ) {
                        std::clog << "sem_post: " << pretty_strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int get(int *value) const
                {
                    if ( sem_getvalue(&_M_sem,value) < 0 ) {
                        std::clog << "sem_getvalue: " << pretty_strerror(errno) << std::endl; 
                        return -1;
                    }
                    return 0;
                }

            private:
                mutable sem_t _M_sem;

        }; 
    };

} // namespace more

#endif /* __SEMAPHORE_HH__ */
