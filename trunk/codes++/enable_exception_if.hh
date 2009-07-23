/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ENABLE_EXCEPTION_IF_HH_
#define _ENABLE_EXCEPTION_IF_HH_ 

#include <iostream>
#include <stdexcept>
#include <typeinfo>
#include <cassert>

#ifndef PP_NARG
/* the so-called __VA_NARG__ (PP_NARG) macro from the thread at 
http://groups.google.com/group/comp.std.c/browse_frm/thread/77ee8c8f92e4a3fb 
 */

#define PP_NARG(...) \
PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
                  _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
                  _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
                  _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
                  _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
                  _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
                  _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
                  _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
63,62,61,60,                   \
59,58,57,56,55,54,53,52,51,50, \
49,48,47,46,45,44,43,42,41,40, \
39,38,37,36,35,34,33,32,31,30, \
29,28,27,26,25,24,23,22,21,20, \
19,18,17,16,15,14,13,12,11,10, \
9,8,7,6,5,4,3,2,1,0 

#define PASTE(a,b)      a ## b
#define XPASTE(a,b)     PASTE(a,b)

#endif

#ifndef throw_

#define throw_1(x)      this->throw_exception(x); return;
#define throw_2(x,r)    this->throw_exception(x); return r;
#define throw_(...)     XPASTE(throw_, PP_NARG(__VA_ARGS__))(__VA_ARGS__)         

#endif

namespace more {

    namespace {

        template <bool value> struct ct_assert;
        template <>
        struct ct_assert<true>
        {
            enum { value = true };
        };
    }

    template <bool value>
    class enable_exception_if // <false> 
    {
        mutable bool _M_value;

    protected:
        ~enable_exception_if()
        {}

    public:
        enable_exception_if()
        : _M_value(true)
        {}

        operator bool() const
        { return _M_value; }

        template <typename T>
        void throw_exception(const T & e) const
        {
            std::clog << "warning: exception error of '" << typeid(T).name() << "': ";
            const std::exception * p = dynamic_cast<const std::exception *>(&e);
            if (p) {
                std::clog << p->what() << '!';
            }
            std::clog << std::endl;
            _M_value = false;        
        }

    };

    template <>
    class enable_exception_if<true>
    {
    protected:
        ~enable_exception_if()
        {}

    public:

        operator bool() const
        { return true; }

        template <typename T>
        void
        throw_exception(const T &e) const
        { 
#ifdef __EXCEPTIONS
            throw e;
#else
            ct_assert<false> exception_enabled_while_fno_exceptions_option_is_set;
#endif
        }

    };

} // namespace more

#endif /* _ENABLE_EXCEPTION_IF_HH_ */
