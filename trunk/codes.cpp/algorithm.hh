/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _ALGORITHM_HH_
#define _ALGORITHM_HH_ 

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <tr1/functional>
namespace std { using namespace std::tr1; }
#else
#include <functional>
#endif

#include <algorithm>
#include <iterator>
#include <vector>
#include <string>

namespace more { 

    // copy_if (inspired to that of HP STL)
    //

    template <typename InputIter, typename OutputIter, typename Pred>
    OutputIter copy_if(InputIter beg, InputIter end, OutputIter dest_beg, Pred op)    
    {
        for(; beg != end; ) {
            if ( op(*beg) )
                *dest_beg++ = *beg;
                ++beg;
        }
        return dest_beg;
    }

    // iota not yet in the standard...  
    //

    template <typename Iter, typename Tp>
    void iota(Iter __first, Iter __last, Tp __value)
    {
        for(; __first != __last; ++__first)
        {
            *__first = __value++;
        }
    }

    // Levenshtein Distance Algorithm: C++ Implementation
    // a generic rewrite of http://www.merriampark.com/ldcpp.htm
    //

    template <typename It, typename Distance >  
    int levenshtein_distance(It __beg, It __end, It __beg2, It __end2, Distance dist)
    {
        typedef typename std::iterator_traits<It>::difference_type difference_type;
        typedef typename std::iterator_traits<It>::value_type value_type;
        typedef std::vector<std::vector<int> > matrix_type;
        
        const difference_type n = std::distance(__beg,  __end);
        const difference_type m = std::distance(__beg2, __end2);
   
        if (!(n && m))
           return n ? : m;

        matrix_type matrix(n+1, std::vector<int>(m+1));
   
        for (int i = 0; i <= n; i++) {
            matrix[i][0]=i;
        }
        for (int j = 0; j <= m; j++) {
            matrix[0][j]=j;
        }

        It __s = __beg;

        for (int i = 1; i <= n; i++, ++__s) {

            It __t = __beg2;

            for (int j = 1; j <= m; j++, ++__t) {

                const int cost  = dist(* __s,* __t);
                const int above = matrix[i-1][j];
                const int left  = matrix[i][j-1];
                const int diag  = matrix[i-1][j-1];
                int cell = std::min(above + 1, std::min(left + 1, diag + cost));

                matrix[i][j] = cell;
            }
        }

        return matrix[n][m];
    }

    template <typename It>  
    int levenshtein_distance(It __beg, It __end, It __beg2, It __end2)
    {
        return levenshtein_distance(__beg, __end, __beg2, __end2, std::not_equal_to<typename std::iterator_traits<It>::value_type >());
    } 

    template<typename CharT, typename Traits, typename Alloc>
    int levenshtein_distance(const std::basic_string<CharT, Traits, Alloc> &s1,
                             const std::basic_string<CharT, Traits, Alloc> &s2)
    {
        return levenshtein_distance(s1.begin(), s1.end(), s2.begin(), s2.end(), std::not_equal_to<CharT>()); 
    }

} // namespace more
#endif /* _ALGORITHM_HH_ */
