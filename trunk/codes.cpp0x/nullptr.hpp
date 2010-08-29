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

#if defined(__GNUC__) && !defined(__GXX_EXPERIMENTAL_CXX0X__) || \
    defined(__GNUC__) && (__GNUC__ == 4 && __CNUC_MINOR__ < 6)  

// c++0x nullptr which can only be assigned to pointers (not yet part of the standard).
// Nicola

namespace
{
    const
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

    } nullptr = {};
}

#endif

#endif /* _MORE_NULLPTR_HPP_ */
