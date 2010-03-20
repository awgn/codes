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

    // logical_xor
    //

    template <typename T1, typename T2>
    inline bool
    logical_xor(const T1 &a, const T2 &b)
    {
        return static_cast<bool>(a) ^ static_cast<bool>(b);
    } 

    // chop
    //

    template <typename T>
    struct chop : public std::unary_function<T,T>
    {
        chop(const T &_min, const T &_max)
        : _M_min(_min), _M_max(_max)
        {}

        T operator()(const T &value) const
        {
            return value < _M_min ? (_M_min) : ( value > _M_max ? _M_max : value );
        }

        T _M_min;
        T _M_max;
    };

    // flipflop predicate 
    //
 
    template <typename Arg>
    struct flipflop : public std::unary_function<Arg, bool>
    {
        template <typename Pred>
        flipflop(const Pred &p)
        : _M_state(false), _M_pred(p)
        {}

        bool
        operator()(const Arg &value) const
        {
            return _M_state = logical_xor( _M_state, _M_pred(value) );
        }

        mutable bool _M_state;
        std::function< bool(Arg) > _M_pred;
    };

    // flipflop2 predicate
    //

    template <typename Arg>
    struct flipflop2 : public std::unary_function<Arg, bool>
    {
        template <typename Pred1, typename Pred2>
        flipflop2(const Pred1 &p1, const Pred2 &p2)
        : _M_state(false), _M_pred1(p1), _M_pred2(p2)
        {}

        bool
        operator()(const Arg &value) const
        {
            return _M_state = _M_state ? logical_xor(_M_state, _M_pred2(value)) :
                                         logical_xor(_M_state, _M_pred1(value)) ; 
        }

        mutable bool _M_state;

        std::function< bool(Arg) > _M_pred1;
        std::function< bool(Arg) > _M_pred2;
    };


    // Levenshtein Distance Algorithm: C++ Implementation
    // a generic rewrite of http://www.merriampark.com/ldcpp.htm
    //

    template <typename It, typename Eq >  
    int levenshtein_distance(It __beg, It __end, It __beg2, It __end2, Eq eq)
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

                const int cost  = !eq(* __s,* __t);
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
        return levenshtein_distance(__beg, __end, __beg2, __end2, std::equal_to<typename std::iterator_traits<It>::value_type >());
    } 

    template<typename CharT, typename Traits, typename Alloc>
    int levenshtein_distance(const std::basic_string<CharT, Traits, Alloc> &s1,
                             const std::basic_string<CharT, Traits, Alloc> &s2)
    {
        return levenshtein_distance(s1.begin(), s1.end(), s2.begin(), s2.end(), std::equal_to<CharT>()); 
    }


} // namespace more
#endif /* _ALGORITHM_HH_ */
