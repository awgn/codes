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

namespace more { namespace expr { 

    // unary expression template...
    //

    template <typename A, typename Op>
    class unary_expr
    {
    public:
        typedef typename A::value_type value_type;

        unary_expr(A arg, Op op = Op())
        : _M_arg(arg),
          _M_op(op)
        {}

        value_type 
        operator()() const 
        {
            return _M_op(_M_arg());
        }

        template <typename C>
        value_type 
        operator()(const C &ctx) const 
        {
            return _M_op(_M_arg(ctx));
        }


        A 
        argument() const
        { return _M_arg; }

        Op
        operand() const
        { return _M_op; } 

    private:
        A _M_arg;
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
        typedef typename equal<typename T1::value_type,typename T2::value_type>::type value_type;

        binary_expr(T1 _l, T2 _r, Op _op = Op() )
        : _M_lhs(_l), _M_rhs(_r), _M_op(_op)
        {}

        value_type
        operator()() const
        {
            return _M_op(_M_lhs(), _M_rhs());
        }

        template <typename C>
        value_type
        operator()(const C &ctx) const
        {
            return _M_op(_M_lhs(ctx), _M_rhs(ctx));
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

    // operand function objects...
    //

    struct op_not
    {
        template <typename T>
        T operator()(T _l) const
        {
            return !static_cast<bool>(_l);
        }
    };

    struct op_or
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return static_cast<bool>(_l) | static_cast<bool>(_r);
        }         
    };

    struct op_and
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return static_cast<bool>(_l) & static_cast<bool>(_r);
        }         
    };

    struct op_xor
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return static_cast<bool>(_l) ^ static_cast<bool>(_r);
        }         
    };

    struct op_eq
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return static_cast<bool>(_l) == static_cast<bool>(_r);
        }         
    };

    struct op_not_eq
    {
        template <typename T>
        T operator()(T _l, T _r) const
        {
            return static_cast<bool>(_l) != static_cast<bool>(_r);
        }         
    };


    template <typename T> struct op_traits;
    template <>
    struct op_traits<op_not>
    {
        static const char * symbol()
        { return "!"; }
    };

    template <>
    struct op_traits<op_or>
    {
        static const char * symbol()
        { return "|"; }
    };
    template <>
    struct op_traits<op_and>
    {
        static const char * symbol()
        { return "&"; }
    };
    template <>
    struct op_traits<op_xor>
    {
        static const char * symbol()
        { return "^"; }
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

    // generators...
    //

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_or>
    operator|(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_or>(_l, _r);
    }

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_and>
    operator&(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_and>(_l, _r);
    }

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_xor>
    operator^(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_xor>(_l, _r);
    }

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_eq>
    operator==(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_eq>(_l, _r);
    }

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_not_eq>
    operator!=(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_not_eq>(_l, _r);
    }

    template <typename T>
    unary_expr<T, op_not>
    operator!(T el)
    {
        return unary_expr<T, op_not>(el);
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
    typename T::value_type 
    eval(T ex)
    {
        return ex();
    }

    // evalute the expression template over a give context
    //

    template <typename T, typename C>
    typename T::value_type 
    eval(T ex, const C & ctx)
    {
        return ex(ctx);
    }

    //////////////////// example ///////////////////////

    // terminal expression-template that
    // represent an integral type.
    //

    template <typename T, T value>
    struct integral
    {
        typedef T value_type;

        T operator()() const
        { return value; }
    };

    template <typename T>
    struct identity_context 
    {
        typedef T value_type;

        T operator()(T value) const
        { return value; }
    };

    template <typename CharT, typename Traits, typename T, T value>
    inline std::basic_ostream<CharT,Traits> &
    operator<< (std::basic_ostream<CharT,Traits> &out, const integral<T,value> &un)
    {
        return out << value; 
    }

    // example: _false and _true integral type.
    //

    static integral<bool, false> _false;
    static integral<bool, true>  _true;
    
    static identity_context<bool> _bool;    // a boolean type that exploit the context...


} // namespace expr 
} // namespace more

#endif /* _EXPRTEMPL_HH_ */
