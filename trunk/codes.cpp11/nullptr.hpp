/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_NULLPTR_HPP_
#define _MORE_NULLPTR_HPP_ 

#if defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ < 6) && !defined(__clang__)

// c++11 nullptr which can only be assigned to pointers (for compilers not supporting it yet).
// Nicola

namespace
{
    struct nullptr_t
    {
        template <typename T>
        operator T *() const
        {
            return 0;
        }

        template <class C, class T>
        operator T C::*() const
        {
            return 0;
        }
    } 
    const nullptr = {};

    template <typename T>
    bool operator==(T *p, nullptr_t)
    {
        return p == (T *)0;
    }

    template <typename T>
    bool operator==(nullptr_t, T *p)
    {
        return p == (T *)0;
    }

    template <typename T>
    bool operator!=(T *p, nullptr_t)
    {
        return !(p == nullptr);
    }

    template <typename T>
    bool operator!=(nullptr_t, T *p)
    {
        return !(p == nullptr);
    }
}

#endif

#endif /* _MORE_NULLPTR_HPP_ */
