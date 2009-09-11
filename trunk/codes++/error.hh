/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ERROR_HH_
#define _ERROR_HH_ 

#include <cstring>
#include <string>
#include <stdexcept>

#include <errcode.h>

namespace more { 

    // threadsafe strerror_r
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

    static inline
    const char *
    strerrorcode(int num)
    {
        if (num < 0 || num > 132)
            throw std::out_of_range("more::strerrorcode"); 
        return errcode_str[num];       
    }

    static inline
    std::string
    pretty_strerror(int num)
    {
#ifdef _REENTRANT
        char store[80];
        return std::string(::strerror_r(num, store, sizeof(store)/sizeof(store[0]))).append(" [").append(more::errcode_str[num]).append("]");
#else
        return std::string(::strerror(num)).append(" [").append(more::errcode_str[num]).append("]");
#endif
    }


    // syscall_error exception
    //

    class syscall_error : public std::exception
    {
        std::string _M_msg;
        int _M_err;

    public:
        explicit syscall_error(const std::string &m, int e = errno)
        : _M_msg( more::strerror(e).append(pretty_strerror(e)).append(" : ").append(m) ),
          _M_err(e)
        {}

        syscall_error(int e = errno)
        : _M_msg( more::strerror(e).append(pretty_strerror(e))),
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
        err_no() const
        { return _M_err; }

    };


} // namespace more

#endif /* _ERROR_HH_ */
