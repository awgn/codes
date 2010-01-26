/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _NULLPTR_HH_
#define _NULLPTR_HH_ 

static struct __nullptr
{
    template <typename T>
    operator T *()
    {
        return 0;
    }

} nullptr;

#endif /* _NULLPTR_HH_ */
