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

    template <typename Arg, typename Op>
    class unary_expr
    {
    public:
        typedef typename Arg::value_type value_type;

        unary_expr(Arg arg, Op op = Op())
        : _M_arg(arg),
        _M_op(op)
        {}

        value_type 
        eval() const 
        {
            return _M_op(_M_arg.eval());
        }

        template <typename C>
        value_type 
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
        typedef typename equal<typename T1::value_type,typename T2::value_type>::type value_type;

        binary_expr(T1 _l, T2 _r, Op _op = Op() )
        : _M_lhs(_l), _M_rhs(_r), _M_op(_op)
        {}

        value_type
        eval() const
        {
            return _M_op(_M_lhs.eval(), _M_rhs.eval());
        }

        template <typename C>
        value_type
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


    // generators...
    //

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_or>
    operator|(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_or>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_logical_or>
    operator||(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_logical_or>(_l, _r);
    }

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_and>
    operator&(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_and>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_logical_and>
    operator&&(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_logical_and>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_xor>
    operator^(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_xor>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_logical_xor>
    logical_xor(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_logical_xor>(_l, _r);
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

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_greater>
    operator>(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_greater>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_greater_eq>
    operator>=(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_greater_eq>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_less>
    operator<(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_less>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_less_eq>
    operator<=(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_less_eq>(_l, _r);
    }
 
    template <typename T>
    unary_expr<T, op_not>
    operator!(T el)
    {
        return unary_expr<T, op_not>(el);
    }
    template <typename T>
    unary_expr<T, op_compl>
    operator~(T el)
    {
        return unary_expr<T, op_compl>(el);
    }

    // +-*/...

    template <typename T1, typename T2>
    binary_expr<T1, T2, op_add>
    operator+(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_add>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_sub>
    operator-(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_sub>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_mul>
    operator*(T1 _l, T2 _r)
    {
        return binary_expr<T1, T2, op_mul>(_l, _r);
    }
    template <typename T1, typename T2>
    binary_expr<T1, T2, op_div>
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
    typename T::value_type 
    eval(T ex)
    {
        return ex.eval();
    }

    // evalute the expression template over a given context
    //

    template <typename T, typename C>
    typename T::value_type 
    eval(T ex, const C & ctx)
    {
        return ex.eval(ctx);
    }

} // namespace expr 
} // namespace more

#endif /* _EXPRTEMPL_HH_ */
