/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */
#ifndef SPRINT_HH
#define SPRINT_HH

#include <string>
#include <iostream>
#include <cstdlib>
#include <macro_template.h>

#ifndef likely
#define likely(x)   __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

namespace more 
{
    static const int SPRINT_BUFFSIZE = 1024;

    static inline std::string sprint(const char *fmt) __attribute__((unused));
    static inline std::string sprint(const char *fmt) 
    {
        char strp[SPRINT_BUFFSIZE];
        int n = snprintf(strp, SPRINT_BUFFSIZE, fmt);
        if (unlikely(n < 0))
            return std::string();
        if (unlikely(n >= SPRINT_BUFFSIZE))
             std::clog << "warning: sprint truncated output\n";
        return std::string(strp);
    }

#define aFunction(N) \
template < MT_REPEAT_ARG(typename T,N) > \
static inline std::string sprint(const char *fmt, MT_REPEAT_ARG2(T,arg_,N)) \
{ \
        char strp[SPRINT_BUFFSIZE]; \
        int n = snprintf(strp, SPRINT_BUFFSIZE, fmt, MT_REPEAT_ARG(arg_,N)); \
        if (unlikely(n < 0)) \
            return std::string(); \
        if (unlikely(n >= SPRINT_BUFFSIZE))  \
             std::clog << "warning: sprint truncated output\n"; \
        return std::string(strp); \
}
 
MT_REPEAT_STATEMENT(aFunction,15);

}

#endif /* SPRINT_HH */

