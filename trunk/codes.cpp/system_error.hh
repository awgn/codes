/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SYSTEM_ERROR_HH_
#define _SYSTEM_ERROR_HH_ 

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cstring>
#include <string>
#include <stdexcept>
#include <cerrno>

#ifdef _REENTRANT
#include <atomicity-policy.hh>
#endif

namespace more { 

    // threadsafe strerror
    //

    static inline
    std::string
    strerror(int num)
    {
#ifdef _REENTRANT
        char store[80];
        return ::strerror_r(num, store, sizeof(store)/sizeof(store[0]));
#else
        return ::strerror(num);
#endif
    }

    // threadsafe gai_strerror
    //

    static inline
    std::string
    gai_strerror(int num)
    {
#ifdef _REENTRANT
        static atomicity::GNU_CXX::mutex m;
        atomicity::GNU_CXX::scoped_lock _lock_(m);
#endif
        return ::gai_strerror(num);
    }

    // system_error exception
    //

    class system_error : public std::exception
    {
        std::string m_msg;
        int m_err;

    public:
        explicit system_error(const std::string &m, int e = errno)
        : m_msg(m), 
          m_err(e)
        {
            m_msg.append(" (").append(strerror(e)).append(")");
        }

        system_error(int e = errno)
        : m_msg(strerror(e)),
          m_err(e)
        {}

        virtual ~system_error() throw()
        {}

        virtual const char*
        what() const throw()
        {
            return m_msg.c_str();
        }

        int
        err_no() const
        { return m_err; }

    };


} // namespace more

#endif /* _SYSTEM_ERROR_HH_ */
