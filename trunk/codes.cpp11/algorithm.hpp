/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_ALGORITHM_HPP_
#define _MORE_ALGORITHM_HPP_ 

#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <functional>

namespace more { 

    // Levenshtein Distance Algorithm: C++ Implementation
    // a generic rewrite of http://www.merriampark.com/ldcpp.htm
    //

    template <typename It, typename Distance >  
    int levenshtein_distance(It __beg, It __end, It __beg2, It __end2, Distance dist)
    {
        typedef std::vector<std::vector<int> > matrix_type;
        
        auto n = std::distance(__beg,  __end);
        auto m = std::distance(__beg2, __end2);
   
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
    inline int levenshtein_distance(It __beg, It __end, It __beg2, It __end2)
    {
        return levenshtein_distance(__beg, __end, __beg2, __end2, std::not_equal_to<typename std::iterator_traits<It>::value_type >());
    } 

    template<typename CharT, typename Traits, typename Alloc>
    inline int levenshtein_distance(const std::basic_string<CharT, Traits, Alloc> &s1,
                             const std::basic_string<CharT, Traits, Alloc> &s2)
    {
        return levenshtein_distance(s1.begin(), s1.end(), s2.begin(), s2.end(), std::not_equal_to<CharT>()); 
    }

} // namespace more

#endif /* _ALGORITHM_HPP_ */
