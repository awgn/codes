/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _FUNCTIONAL_HH_
#define _FUNCTIONAL_HH_ 

#include <tr1_functional.hh>    // more!

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

    // norm (valid for signed and unsigned types)
    //
    
    template <typename Tp>
    struct norm : public std::binary_function<Tp, Tp, Tp>
    {
        Tp
        operator()(const Tp& x, const Tp& y) const
        {
            return std::max(x,y)-std::min(x,y);
        }
    }; 

} // namespace more
#endif /* _FUNCTIONAL_HH_ */
