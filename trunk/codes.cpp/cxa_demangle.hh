/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _CXA_DEMANGLE_HH_
#define _CXA_DEMANGLE_HH_ 

#include <cxxabi.h>

#include <tr1/memory>   

#ifdef _REENTRANT
#include <atomicity-policy.hh>
#endif

#include <string>
#include <cstdlib>
#include <stdexcept>

namespace more { 

    static inline
    std::string
    cxa_demangle(const char *name)
    {
#ifdef _REENTRANT
        static atomicity::GNU_CXX::mutex s_mutex;
        atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
        int status;
        std::tr1::shared_ptr<char> ret(abi::__cxa_demangle(name,0,0, &status), ::free);
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

#endif /* _CXA_DEMANGLE_HH_ */
