/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MUTANT_HH_
#define _MUTANT_HH_ 

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <tr1/type_traits>
namespace std { using namespace std::tr1; }
#else
#include <type_traits>
#endif

namespace more
{
    // compile time assert 
    template <bool> struct mutant_assert;
    template <>
    struct mutant_assert<true>
    {
        enum { value = true };
    };

    struct mutant
    {
        virtual ~mutant() {}

        template <typename T>
        void turn_into(const T & exemplar)
        {
            mutant_assert< std::is_polymorphic<T>::value > is_polymorphic_concept __attribute__ ((unused));
            mutant_assert< std::is_base_of<mutant, T>::value > is_base_concept __attribute__((unused));
            *(void **)this = *(void **)& exemplar;
        }
    };

} // namespace more

#endif /* _MUTANT_HH_ */
