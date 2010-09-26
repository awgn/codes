/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _CXA_DEMANGLE_HPP_
#define _CXA_DEMANGLE_HPP_ 

#include <cxxabi.h>

#include <string>
#include <cstdlib>
#include <stdexcept>
#include <mutex>
#include <memory>    

namespace more { 

    static inline
    std::string
    cxa_demangle(const char *name)
    {
#ifdef _REENTRANT
        static std::mutex _S_mutex;
        std::lock_guard<std::mutex> _L_(_S_mutex);
#endif
        int status;
        std::shared_ptr<char> ret(abi::__cxa_demangle(name,0,0, &status), ::free);
        if (status < 0) {
#ifdef __EXCEPTIONS
            throw std::runtime_error("__cxa_demangle");
#else
            return std::string("?");
#endif
        }
        return std::string(ret.get());
    }

} // namespace more

#endif /* _CXA_DEMANGLE_HPP_ */