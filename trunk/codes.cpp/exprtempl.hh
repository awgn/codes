/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _EXPRTEMPL_HH_
#define _EXPRTEMPL_HH_ 

#include <mtp.hh>       // more!
#include <iostream>

namespace more { namespace expr { 

    // unary expression template...
    //

    template <typename Arg, typename Op>
    class unary_expr
    {
    public:
        typedef typename Arg::expression_type expression_type;

        unary_expr(Arg arg, Op op = Op())
        : _M_arg(arg),
        _M_op(op)
        {}

        expression_type 
        eval() const 
        {
            return _M_op(_M_arg.eval());
        }

        template <typename C>
        expression_type 
        eval(const C &ctx) const 
        {
            return _M_op(_M_arg.eval(ctx));
        }

        Arg 
        argument() const
        { return _M_arg; }

        Op
        operand() const
        { return _M_op; } 

    private:
        Arg _M_arg;
        Op _M_op;
    };

    // ensure the type are equal
    //

    template <typename T1, typename T2> struct equal;
    template <typename T>
    struct equal<T,T> 
    {
        typedef T type; 
    };

    // binary_expr expression template...
    //

    template <typename T1, typename T2, typename Op>
    class binary_expr
    {
    public:
        typedef typename equal<typename T1::expression_type,typename T2::expression_type>::type expression_type;

        binary_expr(T1 _l, T2 _r, Op _op = Op() )
        : _M_lhs(_l), _M_rhs(_r), _M_op(_op)
        {}

        expression_type
        eval() const
        {
            return _M_op(_M_lhs.eval(), _M_rhs.eval());
        }

        template <typename C>
        expression_type
        eval(const C &ctx) const
        {
            return _M_op(_M_lhs.eval(ctx), _M_rhs.eval(ctx));
        }

        T1
        left() const
        { return _M_lhs; }

        T2
        right() const
        { return _M_rhs; }

        Op
        operand() const
        { return _M_op; } 

    private:
        T1 _M_lhs;
        T2 _M_rhs;
        Op _M_op;
    };

    // terminal expression-template that
    // represent an integral type.
    // 

    template <typename T, T value>
    struct integral
    {
        typedef T expression_type;

        T eval() const
        { return value; }

        template <typename C>
        T eval(const C &) const
        { return value; }
    };

    template <typename CharT, typename Traits, typename T, T value>
    inline std::basic_ostream<CharT,Traits> &
    operator<< (std::basic_ostream<CharT,Traits> &out, const integral<T,value> &un)
    {
        return out << value; 
    }

    // _false and _true type...

    namespace 
    {
        integral<bool, false> _false;
        integral<bool, true>  _true;
    }


    // operand function objects...
    //

    struct op_not
    {
        template <typename T>
        bool operator()(T _l) const
        {
            return !_l;
        }
    };
    struct op_compl
    {
        template <typename T>
        T operator()(T _l) const
        {
            return ~_l;
        }
    };
    struct op_or
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return _l | _r;
        }         
    };
    struct op_logical_or
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return _l || _r;
        }         
    };
    struct op_and
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return _l & _r;
        }         
    };
    struct op_logical_and
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return _l && _r;
        }         
    };
    struct op_xor
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return _l ^ _r;
        }         
    };
    struct op_logical_xor
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return static_cast<bool>(_l) ^ static_cast<bool>(_r);
        }         
    };

    struct op_eq
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return  _l == _r;
        }         
    };
    struct op_not_eq
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return _l != _r;
        }         
    };
    struct op_greater
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return  _l > _r;
        }         
    };
    struct op_greater_eq
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return  _l >= _r;
        }         
    };
    struct op_less
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return  _l < _r;
        }         
    };
    struct op_less_eq
    {
        template <typename T>
        bool operator()(T _l, T _r) const
        {
            return  _l <= _r;
        }         
    };

    struct op_add
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return _l +_r;
        }         
    };
    struct op_sub
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return _l - _r;
        }         
    };
    struct op_mul
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return _l * _r;
        }         
    };
    struct op_div
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return _l / _r;
        }         
    };

    // traits...
    //

    template <typename T> struct op_traits;
    template <>
    struct op_traits<op_not>
    {
        static const char * symbol()
        { return "!"; }
    };    
    template <typename T> struct op_traits;
    template <>
    struct op_traits<op_compl>
    {
        static const char * symbol()
        { return "~"; }
    };
    template <>
    struct op_traits<op_or>
    {
        static const char * symbol()
        { return "|"; }
    };    
    template <>
    struct op_traits<op_logical_or>
    {
        static const char * symbol()
        { return "||"; }
    };
    template <>
    struct op_traits<op_and>
    {
        static const char * symbol()
        { return "&"; }
    };    
    template <>
    struct op_traits<op_logical_and>
    {
        static const char * symbol()
        { return "&&"; }
    };
    template <>
    struct op_traits<op_xor>
    {
        static const char * symbol()
        { return "^"; }
    };
    template <>
    struct op_traits<op_logical_xor>
    {
        static const char * symbol()
        { return "XOR"; }
    };

    template <>
    struct op_traits<op_eq>
    {
        static const char * symbol()
        { return "=="; }
    };
    template <>
    struct op_traits<op_not_eq>
    {
        static const char * symbol()
        { return "!="; }
    };
    template <>
    struct op_traits<op_greater>
    {
        static const char * symbol()
        { return ">"; }
    };
    template <>
    struct op_traits<op_greater_eq>
    {
        static const char * symbol()
        { return ">="; }
    };
    template <>
    struct op_traits<op_less>
    {
        static const char * symbol()
        { return "<"; }
    };
    template <>
    struct op_traits<op_less_eq>
    {
        static const char * symbol()
        { return "<="; }
    };
 
    template <>
    struct op_traits<op_add>
    {
        static const char * symbol()
        { return "+"; }
    };
    template <>
    struct op_traits<op_sub>
    {
        static const char * symbol()
        { return "-"; }
    };
    template <>
    struct op_traits<op_mul>
    {
        static const char * symbol()
        { return "*"; }
    };
    template <>
    struct op_traits<op_div>
    {
        static const char * symbol()
        { return "/"; }
    };

    //////////////////
    // generators...
    //

    template <typename T>
    class is_expression_template
    {
        typedef char __one;
        typedef struct { char __arr[2]; } __two;
        
        template <typename C> static __one test(typename C::expression_type *);
        template <typename C> static __two test(...);

    public:
        enum { value = sizeof ( test<T>(0) ) == sizeof(__one) };
    };

    template <typename T, typename Ty>
    class enable_if_expression 
        : public more::mtp::enable_if_c< is_expression_template<T>::value, Ty> 
    {};

    template <typename T1, typename T2, typename Ty>
    class enable_if_expression_2 
        : public more::mtp::enable_if_c< is_expression_template<T1>::value && is_expression_template<T2>::value, Ty > 
    {};

    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_or> >::type
    operator|(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_or>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_logical_or> >::type
    operator||(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_logical_or>(_l, _r);
    }

    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_and> >::type
    operator&(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_and>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_logical_and> >::type
    operator&&(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_logical_and>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_xor> >::type
    operator^(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_xor>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_logical_xor> >::type
    logical_xor(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_logical_xor>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_eq> >::type
    operator==(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_eq>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_not_eq> >::type
    operator!=(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_not_eq>(_l, _r);
    }

    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_greater> >::type
    operator>(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_greater>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_greater_eq> >::type
    operator>=(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_greater_eq>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_less> >::type
    operator<(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_less>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_less_eq> >::type
    operator<=(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_less_eq>(_l, _r);
    }
 
    template <typename T>
    typename enable_if_expression<T,
             unary_expr<T, op_not> >::type
    operator!(T el)
    {
        return unary_expr<T, op_not>(el);
    }
    template <typename T>
    typename enable_if_expression<T,
             unary_expr<T, op_compl> >::type
    operator~(T el)
    {
        return unary_expr<T, op_compl>(el);
    }

    // +-*/...

    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_add> >::type
    operator+(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_add>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_sub> >::type 
    operator-(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_sub>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_mul> >::type
    operator*(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_mul>(_l, _r);
    }
    template <typename T1, typename T2>
    typename enable_if_expression_2<T1,T2, 
             binary_expr<T1, T2, op_div> >::type
    operator/(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_div>(_l, _r);
    }

    // streaming...
    //

    template <typename CharT, typename Traits, typename T, typename Op>
    inline std::basic_ostream<CharT,Traits> &
    operator<< (std::basic_ostream<CharT,Traits> &out, const unary_expr<T,Op> &un)
    {
        return out << op_traits<Op>::symbol() << un.argument();
    }

    template <typename CharT, typename Traits, typename T1, typename T2, typename Op>
    inline std::basic_ostream<CharT,Traits> &
    operator<< (std::basic_ostream<CharT,Traits> &out, const binary_expr<T1,T2,Op> &bin)
    {
        return out << '(' << bin.left() << ' ' << op_traits<Op>::symbol() << ' ' << bin.right() << ')';
    }

    // evaluate the expression template...
    //

    template <typename T>
    typename T::expression_type 
    eval(T exp)
    {
        return exp.eval();
    }

    // evalute the expression template over a given context
    //

    template <typename T, typename C>
    typename T::expression_type 
    eval(T exp, const C & ctx)
    {
        return exp.eval(ctx);
    }

} // namespace expr 
} // namespace more

#endif /* _EXPRTEMPL_HH_ */
