/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TUPLARR_HH_
#define _TUPLARR_HH_ 

#include <tr1/tuple>
#include <tr1/array>

namespace more 
{
    namespace tuplarr_utils 
    {
        template <typename C, typename T, int N >
        struct pushback_policy 
        {
            static void apply(C &cnt, const T &tupl)
            {
                cnt.push_back( std::tr1::get<std::tr1::tuple_size<T>::value-N>(tupl) );
                pushback_policy<C,T,N-1>::apply(cnt,tupl); 
            }
        };

        template <typename C, typename T>
        struct pushback_policy<C,T,0> 
        {
            static void apply(C &cnt, const T &tuple)
            {}
        };
    }

    template <typename C, typename T>
    static inline 
    void container_backinsert_tuplarr(C &cnt, const T &tupl)
    {
        tuplarr_utils::pushback_policy<C,T,std::tr1::tuple_size<T>::value>::apply(cnt,tupl);
    }

}

#endif /* _TUPLARR_HH_ */
