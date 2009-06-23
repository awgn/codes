/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _CHECKED_DELETE_HH_
#define _CHECKED_DELETE_HH_ 

#include <static_assert.hh>

namespace more
{
    template <typename T>
    void checked_delete(T *p)
    {
        static_assert(sizeof(T),pointer_type_must_be_complete);
        delete p;
    }
}

#endif /* _CHECKED_DELETE_HH_ */
