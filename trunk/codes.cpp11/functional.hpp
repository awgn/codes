/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_FUNCTIONAL_HPP_
#define _MORE_FUNCTIONAL_HPP_ 

#include <functional>    
#include <algorithm>
#include <iterator>
#include <vector>
#include <string>
#include <tuple>

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
        : m_min(_min), m_max(_max)
        {}

        T operator()(const T &value) const
        {
            return value < m_min ? (m_min) : ( value > m_max ? m_max : value );
        }

        T m_min;
        T m_max;
    };

    // flipflop predicate 
    //
 
    template <typename Arg>
    struct flipflop : public std::unary_function<Arg, bool>
    {
        template <typename Pred>
        flipflop(const Pred &p)
        : m_state(false), m_pred(p)
        {}

        bool
        operator()(const Arg &value) const
        {
            return m_state = logical_xor( m_state, m_pred(value) );
        }

        mutable bool m_state;
        std::function< bool(Arg) > m_pred;
    };

    // flipflop2 predicate
    //

    template <typename Arg>
    struct flipflop2 : public std::unary_function<Arg, bool>
    {
        template <typename Pred1, typename Pred2>
        flipflop2(const Pred1 &p1, const Pred2 &p2)
        : m_state(false), m_pred1(p1), m_pred2(p2)
        {}

        bool
        operator()(const Arg &value) const
        {
            return m_state = m_state ? logical_xor(m_state, m_pred2(value)) :
                                         logical_xor(m_state, m_pred1(value)) ; 
        }

        mutable bool m_state;

        std::function< bool(Arg) > m_pred1;
        std::function< bool(Arg) > m_pred2;
    };

    // norm (valid for signed and unsigned types)
    
    template <typename Tp>
    struct norm : public std::binary_function<Tp, Tp, Tp>
    {
        Tp
        operator()(const Tp& x, const Tp& y) const
        {
            return std::max(x,y)-std::min(x,y);
        }
    }; 

    // identity functor
    //

    template <typename Tp>
    struct identity : public std::unary_function<Tp,Tp>
    {
        Tp& 
        operator()(Tp& n) const
        { 
            return n; 
        }        
        
        const Tp& 
        operator()(const Tp& n) const
        { 
            return n; 
        }
 
    };

    // select1st, select2nd non-standard functor
    //

    template <typename Tp>
    struct select1st : public std::unary_function<Tp, typename Tp::first_type>
    {
        typename Tp::first_type 
        operator()(Tp n) const
        {
            return n.first; 
        }
    };

    template <typename Tp>
    struct select2nd : public std::unary_function<Tp, typename Tp::second_type>
    {
        typename Tp::second_type 
        operator()(Tp n) const
        {
            return n.second; 
        }
    };


    template <size_t N, typename Tp>
    struct get_th : public std::unary_function<Tp, typename std::tuple_element<N,Tp>::type>
    {
        typename std::tuple_element<N,Tp>::type
        operator()(const Tp &n) const 
        {
            return std::get<N>(n);
        }
    };

    // unary_callable_if
    //

    template <typename Callable>
    struct unary_callable_if 
    : public std::binary_function<bool, typename Callable::argument_type, 
                                        typename Callable::result_type>
    {
        unary_callable_if(Callable f)
        : m_fun(f)
        {}

        typename Callable::result_type
        operator()(bool cond, typename Callable::argument_type x)
        {
            if (cond)
                return m_fun(x);
            return typename Callable::result_type();
        }

    private:
        Callable m_fun;
    };

    template <typename Callable>
    inline unary_callable_if<Callable>
    call_if(Callable n)
    {
        return unary_callable_if<Callable>(n);
    }

} // namespace more

#endif /* _MORE_FUNCTIONAL_HPP_ */
