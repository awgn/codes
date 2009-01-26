/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef MORE_EXCEPTION_HH
#define MORE_EXCEPTION_HH

#include <iostream>
#include <stdexcept>
#include <cxxabi.h>
#include <sysexits.h>
#include <cstring>
#include <errno.h>

extern char *__progname;

namespace more {

    extern void fatal(const std::string &m) throw(); 
    extern void terminate() __attribute__((noreturn));
    extern void unexpected();

    class fatal_error : public std::exception 
    {
        std::string _M_msg;

    public:
        fatal_error(const std::string &m) 
        : _M_msg(m) 
        { }

        virtual 
        ~fatal_error() throw() 
        {}

        virtual const char* 
        what() const throw()  
        { 
            return _M_msg.c_str(); 
        }
    };

    class syscall_error : public std::exception 
    {
        std::string _M_msg;
        int _M_err;

    public:
        syscall_error(const std::string &m, int e = errno) 
        : _M_msg(std::string(m).append(": ").append(strerror(e))), 
          _M_err(e) 
        {}

        syscall_error(int e) 
        : _M_msg(strerror(e)), 
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
        err() const  
        { return _M_err; }

    };

}

#endif /* MORE_EXCEPTION_HH */

