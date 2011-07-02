/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _KEY_VALUE_HPP_
#define _KEY_VALUE_HPP_ 

#include <typemap.hpp>          // more!

#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <functional>
#include <type_traits>
#include <tuple>
#include <limits>
#include <stdexcept>

#define MAP_KEY(t,k)  struct k { \
    typedef std::pair<k,t> type; \
    static const bool has_default = false; \
    static const char * str() \
    { return # k; } \
};

#define MAP_KEY_VALUE(t,k,v)  struct k { \
    typedef std::pair<k,t> type; \
    static const bool has_default = true; \
    static const char * str() \
    { return # k; } \
    static t default_value() \
    { return v; } \
};

//////////////////////////////////
//  key-value config file parser 

namespace more { 

    namespace details {

        template <class CharT, class Traits>
        inline
        std::basic_istream<CharT,Traits> &
        ignore_line(std::basic_istream<CharT,Traits> &__in)
        {
            __in.ignore(std::numeric_limits<std::streamsize>::max(), __in.widen('\n'));
            return __in;
        }
    
        class line_streambuf : public std::streambuf 
        {
            std::streambuf * m_in;
            int m_line;

        public:
            line_streambuf(std::streambuf *in)
            : m_in(in), m_line(1)
            {}

            virtual int_type underflow()
            {
                return  m_in->sgetc();
            }

            virtual int_type uflow()
            {
                int_type c = m_in->sbumpc();
                if ( c == '\n' )
                    m_line++;
                return c;
            }

            int line() const
            { return m_line; }
        };

        template <class CharT, class Traits>
        inline int line_number(std::basic_istream<CharT,Traits> &in)
        {
            line_streambuf * ln = dynamic_cast<line_streambuf *>(in.rdbuf());
            if (ln)
            {
                return ln->line();
            }
            return -1;
        }

        template <typename KEY, typename TYPE, bool has_default>
        struct get_default_value
        {
            static TYPE value()
            { return KEY::default_value(); }
        };
        template <typename KEY, typename TYPE>
        struct get_default_value<KEY, TYPE, false>
        {
            static TYPE value()
            { return TYPE(); }
        };

        // detail::tuple_helper<sizeof...(Ti)>::parse_lexeme(*this, tup) &&

        template <size_t N>
        struct tuple_helper
        {
            template <typename L, typename ...Ti>
            static bool parse_lexeme(L &lexer,  std::tuple<Ti...> &tup)
            {
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
                typename std::tuple_element<sizeof...(Ti)-N,
                                             typename std::tuple<Ti...>   
                                                >::type elem;

                bool ok = lexer.parse_lexeme(elem);
                if (!ok)
                    return false;

                std::get<sizeof...(Ti) - N>(tup) = elem;
                return tuple_helper<N-1>::parse_lexeme(lexer, tup);
            }
        };
        template <>
        struct tuple_helper<1>
        {
            template <typename L, typename ...Ti>
            static bool parse_lexeme(L &lexer,  std::tuple<Ti...> &tup)
            {
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
                typename std::tuple_element<sizeof...(Ti)-1,
                                             typename std::tuple<Ti...>   
                                                >::type elem;
                bool ok = lexer.parse_lexeme(elem);
                if (ok)
                    std::get<sizeof...(Ti) - 1>(tup) = elem;
                return ok;
            }
        };


    } // namespace details

    /////////////////////////////////////////////////////////////////////////////////////////

    template <typename CharT, typename Traits>
    class __lexer 
    {
        std::basic_istream<CharT, Traits> & m_in;
        const std::tuple<bool, char, char> & m_option;

    public:

        std::basic_istream<CharT, Traits> &
        stream() 
        {
            return m_in;
        }

        __lexer(std::basic_istream<CharT, Traits> &in, const std::tuple<bool,char,char> &opt)
        : m_in(in), m_option(opt)
        {}

        ~__lexer()
        {}

        bool _(char c)
        {
            char _c; return (m_in >> std::skipws >> _c && _c == c) ? true : false;
        }

        bool _(const char *s)
        {
            std::string _s; return (m_in >> std::skipws >> _s && _s.compare(s) == 0) ? true : false;
        }

        // very generic parser for types supporting operator>> ...
        //
        template <typename E>
        inline bool parse_lexeme(E &elem)
        { 
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            return m_in >> std::skipws >> elem;
        }        

        // generic parser for "strings"
        //
        inline bool parse_lexeme(std::string &elem)
        {
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            char c; char quote;

            m_in >> std::skipws;

            if ( !(m_in >> c) ) {
                return false;
            }    
            
            m_in >> std::noskipws;    

            if ( c == '"' || c == '\'') {   // quoted string
                quote = c;
                while (m_in >> c && c != quote) {
                    if ( c == '\\') {
                        if ( !(m_in >> c) )
                            break;
                    }
                    elem.push_back(c);
                }
                m_in >> std::skipws;
                if (c != quote) {
                    std::clog << "parse: error at string '" << elem << ": missing quotation mark\n";
                    return false;
                }
            } else { // simple string
                elem.push_back(c); 
                std::string tmp;
                if ( !(m_in >> tmp) ) 
                    return false;
                elem.append(tmp);
                m_in >> std::skipws;
                return true;
            }
            return true;
        }

        // generic parser for boolean
        //
        inline bool parse_lexeme(bool &elem)
        {
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            m_in >> std::noboolalpha;
            if (!(m_in >> elem)) {
                m_in.clear();
                return m_in >> std::boolalpha >> elem;
            }
            return true;
        }

        // parser for generic pairs
        // 
        template <typename T, typename V>
        inline bool parse_lexeme(std::pair<T,V> &elem)
        { 
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            std::string sep; T first; V second;

            bool ok =   _('(')                  &&
                        parse_lexeme(first)     &&
                        parse_lexeme(second)    &&
                        _(')');  
            if (ok)
                elem = std::make_pair(first,second);
            return ok;
        }


        // parser for generic tuple
        // 
        template <typename ...Ti>
        bool parse_lexeme(std::tuple<Ti...> &elem)
        {
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            std::tuple<Ti...> tup;

            bool ok = _('(') &&
                        details::tuple_helper<sizeof...(Ti)>::parse_lexeme(*this, tup) &&
                      _(')');

            if (ok)
                elem = tup;          
            return ok;
        }

        // generic parser for containers that support push_back()
        //
        template <typename E, 
        template <typename _Tp, typename Alloc = std::allocator<_Tp> > class C >
        inline bool parse_lexeme(C<E> &elems)
        {
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            E tmp; bool ok = parse_lexeme(tmp);

            if (ok)
                elems.push_back(tmp);
            return ok;
        }

        // generic parser for associative containers that support insert(std::pair<K,V>)
        //
        template <typename K, typename V,  
        template <typename _Key, typename _Tp,
        typename _Compare = std::less<_Key>,
        typename _Alloc = std::allocator<std::pair<const _Key, _Tp>>> class Cont >
        inline bool parse_lexeme(Cont<K,V> &elems)
        {
#ifdef LEX_DEBUG
            std::cout << __PRETTY_FUNCTION__ << std::endl;
#endif
            std::string sep; K key; V value;

            bool ok = parse_lexeme(key)     && 
                      _("->")               && 
                      parse_lexeme(value);
            if (ok)
                elems.insert( std::make_pair(key,value) );
            return ok;
        }
    };

    template <typename CharT, typename Traits>
    __lexer<CharT, Traits> 
    lexer(std::basic_istream<CharT, Traits> &in, const std::tuple<bool,char,char> &opt)
    {
        return __lexer<CharT, Traits>(in, opt);
    }

    //////////////////////////////////////////////////////////////////////////
    //   parser class

    template <typename ...T> struct key_value_parser;

    template <typename T0, typename ...Ti>
    struct key_value_parser<T0, Ti...>
    {
    public:
        typedef more::type::typemap<typename T0::type, typename Ti::type...>    map_type;
        typedef typename T0::type::first_type                                   key_type;
        typedef typename T0::type::second_type                                  value_type;

        typedef key_value_parser<Ti...>                                         parser_type;

        parser_type  m_parser;
        key_type     m_key;
        value_type   m_value;

        key_value_parser()
        : m_parser(),
          m_key(), m_value(details::get_default_value<key_type, value_type, key_type::has_default>::value()) 
        {}
                          
        key_value_parser(const char *name, 
            const std::tuple<bool, char, char> &mode = std::make_tuple(false, '=', '#')) 
        : m_parser(),
          m_key(), m_value(details::get_default_value<key_type, value_type, key_type::has_default>::value()) 
        {
            if(!this->open(name, mode))
                throw std::runtime_error("key_value_parser");
        }

        virtual ~key_value_parser()
        {}

        //////////////////////////////////////////////////////////////////////////
        // get method

        template <typename Key>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        get() 
        { return get__<Key>(std::integral_constant<int, more::type::index_of<map_type, Key>::value>()); }
        template <typename Key>

        typename std::add_lvalue_reference<
            typename std::add_const<
                typename more::type::get<map_type, Key>::type>::type>::type
        get() const
        { return const_cast<key_value_parser *>(this)->get__<Key>
            (std::integral_constant<int, more::type::index_of<map_type, Key>::value>()); }
        
        template <typename Key, int N>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        get__(std::integral_constant<int,N>) 
        { return m_parser.get__<Key>(std::integral_constant<int, N-1>()); }

        template <typename Key>
        typename std::add_lvalue_reference<value_type>::type
        get__(std::integral_constant<int,0>) 
        { return m_value; } 

    protected:

        //////////////////////////////////////////////////////////////////////////
        // run-time parser 

        template <typename CharT, typename Traits>
        bool parse_value(std::basic_istream<CharT, Traits> &in, const char *fname, const std::string &key, const std::tuple<bool,char,char> &mode)
        { return s_parse_value(in, fname, key, *this, mode); }

        template <typename CharT, typename Traits, typename _T0, typename ..._Ti>
        static bool s_parse_value(std::basic_istream<CharT, Traits> &in, 
                                  const std::string &fname, const std::string &key, 
                                  key_value_parser<_T0, _Ti...> &that, const std::tuple<bool,char,char> &mode)
        {
            if (key == _T0::type::first_type::str()) {
                
                auto lex = lexer(in, mode);

                if (!lex.parse_lexeme(that.m_value) || in.fail() ) {
                    std::clog << fname << ": parse error: key[" << 
                    _T0::type::first_type::str() << "] unexpected argument (line " << 
                    details::line_number(in) << ")" << std::endl;
                    return false;
                }
                return true;
            }
            return s_parse_value(in, fname, key, that.m_parser, mode);
        }

        template <typename CharT, typename Traits>
        static bool s_parse_value(std::basic_istream<CharT, Traits> &in, const std::string &fname, const std::string &key, 
                                     key_value_parser<> &, const std::tuple<bool,char,char> &mode)
        {
            // unknown key-value...
            if (std::get<0>(mode)) {   // strict mode: dump-error 
                std::clog << fname << ": parse error: key[" << key << "] unknown (line " << 
                details::line_number(in) << ")" << std::endl;
                return false;
            }
            // non-strict mode: skip this line
            in >> details::ignore_line;
            return true;
        }

    public:

        bool 
        open(const char *name, 
            const std::tuple<bool, char, char> &mode = std::make_tuple(false, '=', '#')) 
        {
            std::ifstream sc(name);
            if (!sc) {
                std::clog << name << ": parse error: no such file" << std::endl;
                return false;
            }

            details::line_streambuf sb(sc.rdbuf());
            std::istream in(&sb);    
            return open(in, mode, name);
        }

        template <typename CharT, typename Traits>
        bool open(std::basic_istream<CharT, Traits> &si, 
                 const std::tuple<bool, char, char> &mode = std::make_tuple(false, '=', '#'), 
                 const char *name = "unnamed") 
        {
            si.unsetf(std::ios::dec);
            si.unsetf(std::ios::hex);
            si.unsetf(std::ios::oct);

            for(; si ;) {

                std::string key;
                si >> std::noskipws >> std::ws;

                // parse the key 
                //
                char c('\0');
                while (si >> c && !isspace(c) && c != std::get<1>(mode) ) {
                    key.push_back(c);
                }

                // skip comments/empty lines
                //
                if (key.empty() || key[0] == std::get<2>(mode)) {
                    si >> details::ignore_line;
                    continue;
                }

                si >> std::skipws;

                // parse separator ('=')
                //
                if (c != std::get<1>(mode)) {
                    si >> c; 
                    if (c != std::get<1>(mode)) {
                        std::clog << name << ": parse error: key[" << key << "] missing separator '" 
                        << std::get<1>(mode) << "' (line "<< details::line_number(si) << ")" << std::endl;
                        return false;
                    }
                }

                // parse value...
                // 
                if (!parse_value(si, name, key, mode)) 
                    return false;
            }
            // std::cout << "EOF\n";
            return true;
        }
    };

    template <>
    class key_value_parser<> {};

    template<typename T, typename ...Ti>
    inline 
    typename std::add_lvalue_reference<
        typename more::type::get<typename key_value_parser<Ti...>::map_type, T>::type>::type 
    get(key_value_parser<Ti...> &p) 
    {                                           
        return p.get<T>();
    }

    template<typename T, typename ...Ti>
    inline 
    typename std::add_lvalue_reference<
        typename std::add_const<
            typename more::type::get<typename key_value_parser<Ti...>::map_type, T>::type>::type>::type 
    get(const key_value_parser<Ti...> &p) 
    {       
        return p.get<T>();
    }


} // namespace more

#endif /* _KEYVALUE_HPP_ */
