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
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <type_traits>

#define MAP_KEY(t,k)  struct k { \
    typedef std::pair<k,t> type; \
    static const bool has_default = false; \
    static const char * value() \
    { return # k; } \
};

#define MAP_KEY_VALUE(t,k,v)  struct k { \
    typedef std::pair<k,t> type; \
    static const bool has_default = true; \
    static const char * value() \
    { return # k; } \
    static t default_value() \
    { return v; } \
};

using namespace more::type;

//////////////////////////////////
//  key-value config file parser 

namespace more { namespace key_value {

    template <class charT, class Traits>
    inline
    std::basic_istream<charT,Traits> &
    ignore_line(std::basic_istream<charT,Traits> &__in)
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

    template <class charT, class Traits>
    inline int line_number(std::basic_istream<charT,Traits> &in)
    {
        line_streambuf * ln = dynamic_cast<line_streambuf *>(in.rdbuf());
        if (ln)
        {
            return ln->line();
        }
        return -1;
    }

    template <typename KEY, typename TYPE, bool has_default>
    struct get_default
    {
        static TYPE value()
        { return KEY::default_value(); }
    };
    template <typename KEY, typename TYPE>
    struct get_default<KEY, TYPE, false>
    {
        static TYPE value()
        { return TYPE(); }
    };

    template <typename T, bool S> struct block;

    /////////////////////////////////////////////////////////////////////////////////
    // overloaded lex_parse functions must be provided to parse user-defined types
    //      note: specializations do not partecipate to overloading

    template <typename E>
    inline bool lex_parse(std::istream &in, E &elem)
    { 
        return (in >> elem);
    }        

    template <typename T, bool S>
    inline bool lex_parse(std::istream &in, block<T,S> &b)
    { 
        return b.parse(in, "block");
    }

    // generic container that supports push_back()
    //
    template <typename E, 
             template <typename _Tp, typename Alloc = std::allocator<_Tp> > class C >
             inline bool lex_parse(std::istream &in, C<E> &elems)
             {
                 E tmp;
                 if ( lex_parse(in,tmp) ) {
                     elems.push_back(tmp);
                     return true;
                 }
                 return false;
             }

    // generic associative container that supports insert(std::pair<K,V>)
    //
    template <typename K, typename V,  
             template <typename _Key, typename _Tp,
             typename _Compare = std::less<_Key>,
             typename _Alloc = std::allocator<std::pair<const _Key, _Tp> > > class Cont >
             inline bool lex_parse(std::istream &in, Cont<K,V> &elems)
             {
                 K key;
                 V value;

                 if ( !lex_parse(in,key) )
                     return false;

                 std::string sep;
                 if (!(in >> sep) || sep != "=>" )
                     return false;

                 if ( !lex_parse(in,value) )
                     return false;

                 elems.insert( std::make_pair(key,value) );
                 return true;
             }

    // specialization for boolean
    //
    template <>
    inline bool lex_parse<bool>(std::istream &in, bool &elem)
    {
        in >> std::noboolalpha;
        if (!(in >> elem)) {
            in.clear();
            return (in >> std::boolalpha >> elem);
        }
        return true;
    }

    // specialization for strings, " support
    //
    template <>
    inline bool lex_parse<std::string>(std::istream &in, std::string &elem)
    {
        char c;

        if ( !(in >> c) ) {
            return false;
        }    
        in >> std::noskipws;    

        if ( c == '"' ) {   // quoted string
            while (in >> c && c != '"') {
                if ( c == '\\') {
                    if ( !(in >> c) )
                        break;
                }
                elem.push_back(c);
            }
            in >> std::skipws;
            if (c != '"') {
                std::clog << "parse: error at string '" << elem << ": missing quotation." << std::endl;
                return false;
            }
        } else { // simple string
            elem.push_back(c); 
            std::string tmp;
            if ( !(in >> tmp) ) 
                return false;
            elem.append(tmp);
            in >> std::skipws;
            return true;
        }
        return true;
    }
    //////////////////////////////////////////////////////////////////////////
    //   parser class

    template <typename Tm,  bool Strict     = false /*  mode     */, 
                            char Separator  = '='  /* separator */, 
                            char Comment    = '#' /* comment   */ > struct parser;

    template <typename T0, typename ...Ti, bool Strict, char Separator, char Comment>
    struct parser<typemap<T0, Ti...>, Strict, Separator, Comment>
    {
    public:
        typedef typemap<T0, Ti...>         map_type;
        typedef typename T0::first_type    key_type;
        typedef typename T0::second_type   value_type;

        typedef parser<typemap<Ti...>, Strict, Separator, Comment> parser_type;

        parser_type  m_parser;
        key_type     m_key;
        value_type   m_value;

        parser()
        : m_parser(),
        m_key(),
        m_value(get_default<key_type, value_type, key_type::has_default>::value()) 
        {}

        virtual ~parser()
        {}

        //////////////////////////////////////////////////////////////////////////
        // get method

        template <typename Key>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        get() 
        { return __get<Key>(std::integral_constant<int, more::type::index_of<map_type, Key>::value>()); }

        // the following methods cannot be protected...
        //

        template <typename Key, int N>
        typename std::add_lvalue_reference<typename more::type::get<map_type, Key>::type>::type
        __get(std::integral_constant<int,N>) 
        { return m_parser.__get<Key>(std::integral_constant<int, N-1>()); }

        template <typename Key>
        typename std::add_lvalue_reference<value_type>::type
        __get(std::integral_constant<int,0>) 
        { return m_value; } 

    protected:

        //////////////////////////////////////////////////////////////////////////
        // run-time parser 

        bool parse_key_value(std::istream &in, const std::string &fname, const std::string &key)
        { return parse_key_value_(in, fname, key, *this); }

        template <typename _T0, typename ..._Ti, bool _Strict >
        static bool parse_key_value_(std::istream &in, const std::string &fname, const std::string &key, parser<typemap<_T0, _Ti...>,_Strict,Separator,Comment> &that)
        {
            if (key == _T0::first_type::value()) {
                if (!lex_parse(in,that.m_value) || in.fail() ) {
                    std::clog << fname << ": parse error: key[" << 
                    _T0::first_type::value() << "] unexpected argument (line " << 
                    more::key_value::line_number(in) << ")" << std::endl;
                    return false;
                }
                return true;
            }
            return parse_key_value_(in, fname, key, that.m_parser);
        }
        template <bool _Strict>
        static bool parse_key_value_(std::istream &in, const std::string &fname, const std::string &key, parser<typemap<>,_Strict,Separator,Comment> &)
        {
            // unknown key-value...

            if (_Strict) {   // strict mode: dump-error 
                std::clog << fname << ": parse error: key[" << key << "] unknown (line " << 
                more::key_value::line_number(in) << ")" << std::endl;
                return false;
            }

            // non-strict mode: skip this line
            in >> ignore_line;
            return true;
        }

    public:
        bool parse(const std::string &fname)
        {
            std::ifstream sc(fname.c_str());
            if (!sc) {
                std::clog << fname << ": parse error: no such file" << std::endl;
                return false;
            }

            line_streambuf sb(sc.rdbuf());
            std::istream in(&sb);    
            return parse(in, fname);
        }

        bool parse(std::istream &si, const std::string &fname = "unnamed")
        {
            bool block = false;

            si.unsetf(std::ios::dec);
            si.unsetf(std::ios::hex);
            si.unsetf(std::ios::oct);

            for(; si ;) {

                std::string key;
                si >> std::noskipws >> std::ws;

                // parse KEY 
                //
                char c('\0');
                while ( si >> c && !isspace(c) && c != Separator ) {
                    key.push_back(c);
                }

                // skip comments/empty lines
                //
                if (key.empty() || key[0] == Comment ) {
                    si >> ignore_line;
                    continue;
                }

                si >> std::skipws;

                if (key == "{") {
                    if (block) {
                        std::clog << fname << ": parse error: { nested block are not supported (line " << 
                        more::key_value::line_number(si) << ")" << std::endl;
                        return false;
                    }
                    block = true;
                    continue;
                }

                if (key == "}") {
                    if (block)
                        break;
                    std::clog << fname << ": parse error: expected `{' before the end-of-block } (line "<< 
                    more::key_value::line_number(si) << ")" << std::endl;
                    return false;
                }

                // parse Separator ('=')
                if ( c != Separator ) {
                    si >> c; 
                    if ( c != Separator ) {
                        std::clog << fname << ": parse error: key[" << key << "] missing separator '" 
                        << Separator << "' (line "<< more::key_value::line_number(si) << ")" << std::endl;
                        return false;
                    }
                }

                // parse value... 
                if ( !parse_key_value(si, fname, key) ) 
                    return false;
            }
            // std::cout << "EOF\n";
            return true;
        }
    };

    template <bool Strict, char Separator, char Comment>
    class parser< typemap<>, Strict, Separator, Comment> 
    {};

    template <typename T, bool Strict = false> 
    struct block : public parser<T, Strict, ':', '#'> 
    {}; 

} // namespace key_value
} // namespace more

#endif /* _KEYVALUE_HPP_ */
