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

#ifndef __GXX_EXPERIMENTAL_CXX0X__
namespace more {

    struct nullptr_t
    {
        template <typename T>
        operator T *()
        {
            return 0;
        }
    };

}
#define nullptr more::nullptr_t()
#endif

#endif /* _NULLPTR_HH_ */