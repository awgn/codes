/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _DECLTYPE_HH_
#define _DECLTYPE_HH_ 

// ...sometimes macros are not that evil. Nicola

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#define decltype(x) typeof(x)
#else
#define typeof(x) decltype(x)
#endif

#endif /* _DECLTYPE_HH_ */
