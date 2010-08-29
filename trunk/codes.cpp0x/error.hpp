/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_ERROR_HPP_
#define _MORE_ERROR_HPP_ 

#include <errcode.h>

#include <cstring>
#include <string>
#include <stdexcept>
#include <mutex>

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

    // threadsafe pretty_strerror
    //

    static inline
    std::string
    pretty_strerror(int num)
    {
#ifdef _REENTRANT
        char store[80];
        return std::string(::strerror_r(num, store, sizeof(store)/sizeof(store[0]))).append(" [").append(more::strerrcode(num)).append("]");
#else
        return std::string(::strerror(num)).append(" [").append(more::strerrcode(num)).append("]");
#endif
    }

    // threadsafe gai_strerror
    //

    static inline
    std::string
    gai_strerror(int num)
    {
#ifdef _REENTRANT
        static std::mutex m;
        std::lock_guard<std::mutex> _lock_(m);
#endif
        return ::gai_strerror(num);
    }

    static inline
    std::string
    pretty_gai_strerror(int num)
    {
#ifdef _REENTRANT
        static std::mutex m;
        std::lock_guard<std::mutex> _lock_(m);
#endif
        return std::string(::gai_strerror(num)).append(" [").append(more::gai_strerrcode(num)).append("]");
    }

    // syscall_error exception
    //

    class syscall_error : public std::exception
    {
        std::string _M_msg;
        int _M_err;

    public:
        explicit syscall_error(const std::string &m, int e = errno)
        : _M_msg(m), 
          _M_err(e)
        {
            _M_msg.append(": ").append(pretty_strerror(e));
        }

        syscall_error(int e = errno)
        : _M_msg( pretty_strerror(e) ),
          _M_err(e)
        {}

        virtual ~syscall_error() throw()
        {}

        virtual const char*
        what() const throw()
        {
            return _M_msg.c_str();
        }

        int
        code() const
        { return _M_err; }

    };

} // namespace more

#endif /* _MORE_ERROR_HPP_ */
