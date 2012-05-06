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

#include <system_error.hh>  // more!
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
                : m_sem()
                {
                    if ( this->init(value, pshared) < 0 )
                        throw more::system_error("sem_init", errno);
                }

                ~semaphore()
                {
                    if( sem_destroy(&m_sem) < 0)
                       std::clog << "sem_destroy: " << more::strerror(errno) << std::endl; 
                }

                int init(unsigned int value, int pshared)          
                {      
                    return sem_init(&m_sem, pshared, value);
                }
                   
                int wait() const
                {
                    if ( sem_wait(&m_sem) < 0 ) {
                        std::clog << "sem_wait: " << more::strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int trywait() const
                {
                    if ( sem_trywait(&m_sem) < 0 ) {
                        std::clog << "sem_wait: " << more::strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int timedwait(const struct timespec *abs_timeout) const
                {
                    if ( sem_timedwait(&m_sem, abs_timeout) < 0 ) {
                        std::clog << "sem_timedwait: " << more::strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int post()
                {
                    if ( sem_post(&m_sem) < 0 ) {
                        std::clog << "sem_post: " << more::strerror(errno) << std::endl;
                        return -1;
                    } 
                    return 0;
                }

                int get(int *value) const
                {
                    if ( sem_getvalue(&m_sem,value) < 0 ) {
                        std::clog << "sem_getvalue: " << more::strerror(errno) << std::endl; 
                        return -1;
                    }
                    return 0;
                }

            private:
                mutable sem_t m_sem;

        }; 
    };

} // namespace more

#endif /* __SEMAPHORE_HH__ */
