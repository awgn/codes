/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_GOTOPT_HPP_
#define _MORE_GOTOPT_HPP_

#include <stdexcept>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <tuple>
#include <map>

#include <lexical_cast.hpp>      // more!
#include <exprtempl.hpp>         // more!

//////////////////////////////////////
// ... and now getopt() go home !!!
//////////////////////////////////////

using std::make_tuple;
using std::make_pair;

namespace more { namespace gotopt {

    using namespace more::expr;

    class gotopt_error : public std::runtime_error
    {
    public:
        explicit gotopt_error(const std::string &m)
        : std::runtime_error(m)
        {}

        explicit gotopt_error(const std::exception &e)
        : std::runtime_error(e.what())
        {}

        virtual ~gotopt_error() throw()
        {}
    };

    // gotopt option decriptor
    //

    struct option
    {
        char        opt;
        const char *name;
        int         args;
        const char *descr;

        option(char _opt = char(), const char *_name = nullptr, int _args = 0, const char *_descr = nullptr)
        : opt(_opt), name(_name), args(_args), descr(_descr)
        {}

        option(const char *section)
        : opt(), name(), args(), descr(section)
        {}
    };

    // print help...
    //

    template <typename T>
    std::string usage(const std::string &prolog, const T &options,
                      const std::string & epilog = std::string())
    {
        std::stringstream out;
        out << prolog << std::endl;

        int maxlen = 16;
        for(auto & option : options)
        {
            maxlen = std::max(maxlen, option.name ? static_cast<int>(strlen(option.name)) : 0);
        }

        for(auto & option : options)
        {
            if (option.opt == '\0' &&
                option.descr == nullptr)
            {
                out << std::endl;
                continue;
            }

            if (option.opt == '\0')
            {
                out << option.descr << std::endl;
                continue;
            }

            std::string opt;
            opt.reserve(80);

            if (option.opt > 0)
                opt.append("   -").append(1,option.opt);
            else
                opt.append("       ");

            if (option.name) {
                if (option.opt > 0)
                    opt.append(", ");
                opt.append("--").append(option.name);
            }

            if (option.args)
                opt.append(" ARG");

            out << std::setw(maxlen+16) << std::left << opt <<
                (option.descr ? option.descr : "") << std::endl;
        }

        if (!epilog.empty())
            out << epilog;
        out << std::endl;

        return out.str();
    }

    template <typename T, typename C>
    inline void validate_expression(const T &tupl, const C &ctx);

    // parser class
    //

    class parser
    {
    public:
        typedef std::vector<std::string>::const_iterator        const_iterator;
        typedef std::map<std::string, option>::const_iterator   option_iterator;

    private:
        std::vector<std::string>        m_args;
        std::vector<bool>               m_context;
        std::map<std::string, option>   m_mopt;
        const_iterator                  m_it;
#ifndef NDEBUG
        int                             m_argnum;
#endif

    public:

        // iterators...
        //
        const_iterator
        cbegin() const
        {
            return m_args.cbegin();
        }

        const_iterator
        current() const
        {
            return m_it;
        }

        const_iterator
        cend() const
        {
            return m_args.cend();
        }

        size_t size() const
        {
           return static_cast<size_t>(std::distance(current(), cend()));
        }

        void reset()
        {
            m_it = m_args.cbegin();
        }

        // constructors...
        //

        template <typename T, typename P>
        parser(T it, T end, const P & opt)
        : m_args()
        , m_context(256,false)
        , m_mopt()
        , m_it()
#ifndef NDEBUG
        , m_argnum(0)
#endif
        {
            for(; it != end; ++it)
            {
                std::string s(*it);

                if ( s.size() > 2 && s[0] == '-' && s[1] != '-')
                {
                    m_args.push_back(std::string(s.c_str(), s.c_str()+2));
                    m_args.push_back(std::string(s.c_str()+ 2));
                    continue;
                }
                m_args.push_back(std::move(s));
            }

            m_it = m_args.begin();

            // load the map...

            for(unsigned int i = 0; i < sizeof(opt)/sizeof(opt[0]) ; i++)
            {
                if (opt[i].opt > 0)
                    m_mopt[ std::string("-").append(1, opt[i].opt) ] = opt[i];
                if (opt[i].name)
                    m_mopt[ std::string("--").append(opt[i].name) ] = opt[i];
            }
        }

        ~parser() = default;

        // callable object implementation
        //

        char operator()()
        {
            return operator()(std::make_tuple(_true));
        }

        template <typename T>
        char operator()(const T & exp)
        {
            assert( m_argnum == 0 || !"INTERNAL ERROR: some argument not parsed!");

            // stop parsing at the end of the range, or at the first non-option encountered
            //

            if (m_it == m_args.end() || !is_option(*m_it))
            {
                // alidate the logic:
                //
                validate_expression(exp, m_context);
                return 0;
            }

            // try to find the option pointed by m_it...
            //

            option_iterator cur = m_mopt.find(*m_it);
            if ( cur == m_mopt.end() )
                throw std::runtime_error( std::string("invalid option: ").append(*m_it) );

            // ensure the expected arguments are available...
            //

            int i = 0;
            for(const_iterator arg = m_it+1 ; i < cur->second.args && arg != m_args.end(); ++arg, ++i)
            {
                if ( m_mopt.find(*arg) != m_mopt.end())
                    break;
            }

            if ( i != cur->second.args )
                throw std::runtime_error(std::string(*m_it)
                                         .append(" requires ")
                                         .append(more::lexical_cast<std::string>(cur->second.args))
                                         .append(cur->second.args > 1 ? " arguments" : " argument"));
#ifndef NDEBUG
            m_argnum = cur->second.args;
#endif
            // update the context with the current opt
            //

            char r = m_mopt[*m_it++].opt;
            m_context[static_cast<size_t>(r)] = true;
            return r;
        }

        // get the argument...
        //

        template <typename T>
        T optarg()
        {
            if (m_it == m_args.end())
                throw std::runtime_error("no more arguments available");
#ifndef NDEBUG
            m_argnum--;
#endif
            return more::lexical_cast<T>(*m_it++);
        }

        size_t
        optind() const
        {
            return static_cast<size_t>(std::distance(m_args.begin(), m_it));
        }

    private:

        bool
        is_option(const std::string &s) const
        {
            std::string::size_type n = s.size();
            if ( n == 2 && s[0]=='-' && std::isalnum(s[1]) )
                return true;
            if ( n >  2 && s[0] == '-' && s[1] == '-' )
                return true;
            return false;
        }
    };

    //////////////////////////////////////
    // opt() final expression template...
    //

    template <char V>
    struct opt
    {
        typedef bool expression_type;

        template <typename T>
        bool eval(const T &ctx) const
        {
            // T is supposed to be an associative container such as:
            //   std::map<char, bool> or std::vector<bool> using char as index...
            //

            return ctx[V];
        }
    };

    template <typename CharT, typename Traits, char V>
    inline std::basic_ostream<CharT,Traits> &
    operator<< (std::basic_ostream<CharT,Traits> &out, const opt<V> &)
    {
        return out << V;
    }

    // aliases: special generators/operators ...
    //

    // _a >> _b ( _a implies _b )
    //
    //   if _a is true then _b must be true.
    //

    template <typename T1, typename T2>
    typename more::expr::enable_if_binary<T1,T2,
        more::expr::binary_expr<
            typename more::expr::unary_expr< T1, more::expr::op_not >,
                     T2,
                     more::expr::op_logic_or > >::type
    operator>>(T1 _l, T2 _r)
    {
        return !_l || _r;
    }

    // _a % _b (mutex)
    //
    //   either _a or _b can be true, in mutual exclusion
    //

    template <typename T1, typename T2>
    typename more::expr::enable_if_binary<T1,T2,
        more::expr::unary_expr<
            typename more::expr::binary_expr<T1, T2, more::expr::op_logic_and>,
            more::expr::op_not > >::type
    operator%(T1 _l, T2 _r)
    {
        return !(_l && _r);
    }

    // recursive validation of tuple of expression templates
    //

    template <typename T, typename C>
    void check_expression(const T elem, const C &ctx);

    template <typename T, int N> struct recursive_evaluation;
    template <typename T>
    struct recursive_evaluation<T,0>
    {
        template <typename C>
        static bool apply(const T &, const C &)
        {
            return true;
        }
    };
    template <typename T, int N>
    struct recursive_evaluation
    {
        template <typename C>
        static bool apply(const T &tupl, const C &ctx)
        {
            check_expression(std::get< std::tuple_size<T>::value - N >(tupl), ctx);
            return recursive_evaluation<T, N-1>::apply(tupl,ctx);
        }
    };


    template <typename T, typename C>
    void check_expression(const T elem, const C &ctx)
    {
        if (!more::expr::eval(elem, ctx))
        {
            std::stringstream in; in << "Assert " <<  elem << " failed!";
            throw gotopt_error(in.str());
        }
    }
    template <typename T1, typename T2, typename C>
    void check_expression(const std::pair<T1,T2> &elem, const C &ctx)
    {
        if (!more::expr::eval(elem.first,ctx))
        {
            throw gotopt_error(elem.second);
        }
    }

    template <typename T, typename C>
    inline void validate_expression(const T &tupl, const C &ctx)
    {
        recursive_evaluation<T, std::tuple_size<T>::value >::apply(tupl, ctx);
    }

    ///////////////////////////////
    // predefined options: abc...

    namespace
    {
        // options...

        opt<'a'> _a = opt<'a'>();
        opt<'b'> _b = opt<'b'>();
        opt<'c'> _c = opt<'c'>();
        opt<'d'> _d = opt<'d'>();
        opt<'e'> _e = opt<'e'>();
        opt<'f'> _f = opt<'f'>();
        opt<'g'> _g = opt<'g'>();
        opt<'h'> _h = opt<'h'>();
        opt<'i'> _i = opt<'i'>();
        opt<'j'> _j = opt<'j'>();
        opt<'k'> _k = opt<'k'>();
        opt<'l'> _l = opt<'l'>();
        opt<'m'> _m = opt<'m'>();
        opt<'n'> _n = opt<'n'>();
        opt<'o'> _o = opt<'o'>();
        opt<'p'> _p = opt<'p'>();
        opt<'q'> _q = opt<'q'>();
        opt<'r'> _r = opt<'r'>();
        opt<'s'> _s = opt<'s'>();
        opt<'t'> _t = opt<'t'>();
        opt<'u'> _u = opt<'u'>();
        opt<'v'> _v = opt<'v'>();
        opt<'w'> _w = opt<'w'>();
        opt<'x'> _x = opt<'x'>();
        opt<'y'> _y = opt<'y'>();
        opt<'z'> _z = opt<'z'>();
        opt<'A'> _A = opt<'A'>();
        opt<'B'> _B = opt<'B'>();
        opt<'C'> _C = opt<'C'>();
        opt<'D'> _D = opt<'D'>();
        opt<'E'> _E = opt<'E'>();
        opt<'F'> _F = opt<'F'>();
        opt<'G'> _G = opt<'G'>();
        opt<'H'> _H = opt<'H'>();
        opt<'I'> _I = opt<'I'>();
        opt<'J'> _J = opt<'J'>();
        opt<'K'> _K = opt<'K'>();
        opt<'L'> _L = opt<'L'>();
        opt<'M'> _M = opt<'M'>();
        opt<'N'> _N = opt<'N'>();
        opt<'O'> _O = opt<'O'>();
        opt<'P'> _P = opt<'P'>();
        opt<'Q'> _Q = opt<'Q'>();
        opt<'R'> _R = opt<'R'>();
        opt<'S'> _S = opt<'S'>();
        opt<'T'> _T = opt<'T'>();
        opt<'U'> _U = opt<'U'>();
        opt<'V'> _V = opt<'V'>();
        opt<'W'> _W = opt<'W'>();
        opt<'X'> _X = opt<'X'>();
        opt<'Y'> _Y = opt<'Y'>();
        opt<'Z'> _Z = opt<'Z'>();
    }

} // namespace gotopt
} // namespace more

#endif /* _MORE_GOTOPT_HPP_ */
