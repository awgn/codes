/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _GOTOPT_H_
#define _GOTOPT_H_ 

#include <lexical_cast.hh>      // more!
#include <exprtempl.hh>         // more!

#include <tr1/tuple>         

#include <stdexcept>
#include <iomanip>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <map>

//////////////////////////////////////
// ... and now getopt() go home !!! 
//////////////////////////////////////


using std::tr1::make_tuple;
using std::make_pair;

namespace more { namespace gotopt { 

    using namespace more::expr;

    // gotopt option decriptor
    //

    struct option
    {
        char        opt;
        const char *name;
        int         args;
        const char *description;

        option()
        : opt(), name(), args(), description()
        {}

        option(char _opt, const char *_name, int _args, const char *_descr = NULL)
        : opt(_opt), name(_name), args(_args), description(_descr)
        {}

        option(const char *section)
        : opt(), name(), args(), description(section)
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

        long unsigned int maxlen = 0;
        for(unsigned int i = 0; i < sizeof(options)/sizeof(options[0]) ; i++)
        {
            maxlen = std::max(maxlen, options[i].name ? strlen(options[i].name) : 0UL);    
        }
        maxlen = std::max(maxlen, 16UL); 

        for(unsigned int i = 0; i < sizeof(options)/sizeof(options[0]) ; i++)
        {
            if (options[i].opt) {

                if (options[i].opt > 0)
                    out << "   -" << options[i].opt  << ",";
                else
                    out << "      ";

                out << ( options[i].name ? "  --" : "    " ) << std::setw(maxlen+5) << std::left << ( options[i].name ? : "" ) <<
                (options[i].description ? : "") << std::endl;
                continue;
            }

            if (options[i].description) {
                out << options[i].description << std::endl;
                continue;
            }

            out << std::endl;
        }
        if (!epilog.empty())
            out << epilog << std::endl;
    
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

        const_iterator
        begin() const
        {
            return m_args.begin();
        }

        const_iterator
        current() const
        {
            return m_it;
        }

        const_iterator
        end() const
        {
            return m_args.end();
        }

        // constructors...
        //

        template <typename T, typename P>
        parser(T beg, T end, const P & opt)
        : m_args(beg, end), m_context(256,false), m_mopt(), m_it(m_args.begin()) 
#ifndef NDEBUG
        , m_argnum(0)
#endif 
        {
            // load the map...
            //
            for(unsigned int i = 0; i < sizeof(opt)/sizeof(opt[0]) ; i++)
            {
                if (opt[i].opt > 0) 
                    m_mopt[ std::string("-").append(1, opt[i].opt) ] = opt[i];
                if (opt[i].name)
                    m_mopt[ std::string("--").append(opt[i].name) ]  = opt[i];
            }
        }

        ~parser()
        {}

        // callable object implementation 
        //

        char operator()()
        {
            return operator()(std::tr1::make_tuple(_true));
        }

        template <typename T>
        char operator()(const T & exp)
        {
            assert( m_argnum == 0 || !"INTERNAL ERROR: some argument not parsed!");

            // stop paring at the end of the range, or at the first non-option encountered
            //

            if (m_it == m_args.end() || !is_option(*m_it)) 
            {
                // validate the logic:
                //

                validate_expression(exp, m_context);
                return 0;
            }

            // try to find the option pointed by m_it...
            //

            option_iterator cur = m_mopt.find(*m_it);
            if ( cur == m_mopt.end() )
                throw std::runtime_error( std::string("invalid option: ").append(*m_it) );


            // ensure all the arguments are available...
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

            unsigned char r = std::abs(m_mopt[*m_it++].opt);
            m_context[r] = true;
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

        unsigned int 
        optind() const
        {
            return std::distance(m_args.begin(), m_it);
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

        opt()
        {}

        ~opt()
        {}

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

    // _a >> _b ( _a imply _b ) 
    //
    //   if _a is true then _b must be true.
    //
       
    template <typename T1, typename T2>
    typename more::expr::enable_if_expression_2<T1,T2,
        more::expr::binary_expr<
            typename more::expr::unary_expr< T1, more::expr::op_not >, 
                     T2, 
                     more::expr::op_or > >::type
    operator>>(T1 _l, T2 _r)
    {
        return (!_l) | _r;
    } 

    // _a % _b (mutex)
    //
    //   either _a or _b can be true, in mutual exclusion
    //

    template <typename T1, typename T2>
    typename more::expr::enable_if_expression_2<T1,T2,
        more::expr::unary_expr< 
            typename more::expr::binary_expr<T1, T2, more::expr::op_and>, 
            more::expr::op_not > >::type
    operator%(T1 _l, T2 _r)
    {
        return !(_l & _r); 
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
            check_expression(std::tr1::get< std::tr1::tuple_size<T>::value - N >(tupl), ctx);
            return recursive_evaluation<T, N-1>::apply(tupl,ctx);
        }
    };

    template <typename T, typename C>
    void check_expression(const T elem, const C &ctx)
    {
        if (!more::expr::eval(elem, ctx)) 
        {
            std::stringstream in; in << "Assert " <<  elem << " failed!";
            throw std::runtime_error(in.str());
        }
    }

    template <typename T1, typename T2, typename C>
    void check_expression(const std::pair<T1,T2> &elem, const C &ctx)
    {
        if (!more::expr::eval(elem.first,ctx)) 
        {
            throw std::runtime_error(elem.second);
        }
    }

    template <typename T, typename C>
    inline void validate_expression(const T &tupl, const C &ctx)
    {
        recursive_evaluation<T, std::tr1::tuple_size<T>::value >::apply(tupl, ctx);        
    }

    ///////////////////////////////
    // predefined options: abc...

    namespace
    {
        // options...

        opt<'a'> _a;
        opt<'b'> _b;
        opt<'c'> _c;
        opt<'d'> _d;
        opt<'e'> _e;
        opt<'f'> _f;
        opt<'g'> _g;
        opt<'h'> _h;
        opt<'i'> _i;
        opt<'j'> _j;
        opt<'k'> _k;
        opt<'l'> _l;
        opt<'m'> _m;
        opt<'n'> _n;
        opt<'o'> _o;
        opt<'p'> _p;
        opt<'q'> _q;
        opt<'r'> _r;
        opt<'s'> _s;
        opt<'t'> _t;
        opt<'u'> _u;
        opt<'v'> _v;
        opt<'w'> _w;
        opt<'x'> _x;
        opt<'y'> _y;
        opt<'z'> _z;

        opt<'A'> _A;
        opt<'B'> _B;
        opt<'C'> _C;
        opt<'D'> _D;
        opt<'E'> _E;
        opt<'F'> _F;
        opt<'G'> _G;
        opt<'H'> _H;
        opt<'I'> _I;
        opt<'J'> _J;
        opt<'K'> _K;
        opt<'L'> _L;
        opt<'M'> _M;
        opt<'N'> _N;
        opt<'O'> _O;
        opt<'P'> _P;
        opt<'Q'> _Q;
        opt<'R'> _R;
        opt<'S'> _S;
        opt<'T'> _T;
        opt<'U'> _U;
        opt<'V'> _V;
        opt<'W'> _W;
        opt<'X'> _X;
        opt<'Y'> _Y;
        opt<'Z'> _Z;

        opt<'0'> _0;
        opt<'1'> _1;
        opt<'2'> _2;
        opt<'3'> _3;
        opt<'4'> _4;
        opt<'5'> _5;
        opt<'6'> _6;
        opt<'7'> _7;
        opt<'8'> _8;
        opt<'9'> _9;

    }

} // namespace gotopt
} // namespace more

#endif /* _GOTOPT_H_ */
