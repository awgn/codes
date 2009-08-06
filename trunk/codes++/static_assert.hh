/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef STATIC_ASSERT_HH
#define STATIC_ASSERT_HH

namespace more
{
    ///////////////////////////////
    // compiletime assert ala Loki.

    template <bool> struct compile_time_assert;
    template <>
    struct compile_time_assert<true>
    {
        enum { value = true };
    };
}

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#define static_assert(v,tok) more::compile_time_assert<(v)> tok __attribute__((unused))
#else
#define static_assert(v,tok) static_assert((v), #tok)
#endif

#include <cassert>
#include <stdexcept>

/* from glibc: This prints an "Assertion failed" message and aborts.  */

__BEGIN_DECLS

extern void __assert_fail (__const char *__assertion, __const char *__file, unsigned int __line, __const char *__function)
     __THROW __attribute__ ((__noreturn__));

__END_DECLS

# define perm_assert(expr) \
  (__ASSERT_VOID_CAST ((expr) ? 0 :                       \
               (__assert_fail (__STRING(expr), __FILE__, __LINE__,    \
                       __ASSERT_FUNCTION), 0)))

#ifdef NDEBUG
#define safe_assert(expr) \
if (__builtin_expect(!(expr),0))\
    throw std::runtime_error(__FILE__ ": safe_assert: Assertion `" __STRING(expr) "' failed."  )
#else
#define safe_assert(expr) assert(expr)
#endif

# if defined __cplusplus ? __GNUC_PREREQ (2, 6) : __GNUC_PREREQ (2, 4)
#   define __ASSERT_FUNCTION    __PRETTY_FUNCTION__
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __ASSERT_FUNCTION    __func__
#  else
#   define __ASSERT_FUNCTION    ((__const char *) 0)
#  endif
# endif

#endif /* STATIC_ASSERT_HH */
