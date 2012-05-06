/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _KV_FILE_HH_
#define _KV_FILE_HH_ 

#include <typemap.hh>           // more!
#include <iomanip.hh>           // more!
#include <lnistreambuf.hh>      // more!

#include <tr1/type_traits> 

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

//////////////////////////////////
//  key-value config file parser 

namespace more { namespace kv {

    template <int n>
    struct int2type
    {
        enum { value = n };
    };

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

    template <typename T, 
             bool Strict     = false /* strict mode */, 
             char Separator  = '='  /* separator */, 
             char Comment    = '#' /* comment */ >
     struct parser
     {
     public:
         typedef typename T::key key_type;
         typedef typename T::value value_type;

         typedef parser<typename T::next, Strict, Separator, Comment> map_type;

         map_type     m_map;
         key_type     m_key;
         value_type   m_value;

         parser()
         : m_map(),
         m_key(),
         m_value(get_default<key_type, value_type, key_type::has_default>::value()) 
         {}

         virtual ~parser()
         {}

         //////////////////////////////////////////////////////////////////////////
         // compile-time get

         template <typename K>
         typename std::tr1::add_reference< typename more::TM::get<K, T>::type>::type
         get() 
         { return __get<K>(int2type<more::TM::index_of<K, T>::value >()); }

         template <typename K, int n>
         typename std::tr1::add_reference<typename more::TM::get<K, T>::type>::type
         __get(int2type<n>) 
         { return m_map.__get<K>(int2type<n-1>()); }

         template <typename K>
         typename std::tr1::add_reference<value_type>::type
         __get(int2type<0>) 
         { return m_value; } 

     protected:
         //////////////////////////////////////////////////////////////////////////
         // run-time parser 

         bool parse_key_value(std::istream &in, const std::string &fname, const std::string &key)
         { return __parse(in, fname, key, *this); }

         template <typename U, bool _Strict >
         static bool __parse(std::istream &in, const std::string &fname, const std::string &key, parser<U,_Strict,Separator,Comment> &that)
         {
             if (key == U::key::value()) {
                 if (!lex_parse(in,that.m_value) || in.fail() ) {
                     std::clog << fname << ": parse error: key[" << 
                     U::key::value() << "] unexpected argument (line " << 
                     more::line_number(in) << ")" << std::endl;
                     return false;
                 }
                 return true;
             }
             return __parse(in, fname, key, that.m_map);
         }
         template <bool _Strict>
         static bool __parse(std::istream &in, const std::string &fname, const std::string &key, parser<more::TM::null,_Strict,Separator,Comment> &)
         {
             // unknown key-value...

             if (_Strict) {   // strict mode: dump-error 
                 std::clog << fname << ": parse error: key[" << key << "] unknown (line " << 
                 more::line_number(in) << ")" << std::endl;
                 return false;
             }

             // non-strict mode: skip this line
             
             in >> more::ignore_line;
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

             lnistreambuf sb(sc.rdbuf());
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

                 char c('\0');
                 while ( si >> c && !isspace(c) && c != Separator ) {
                     key.push_back(c);
                 }

                 // skip comments/empty lines

                 if (key.empty() || key[0] == Comment ) {
                     si >> more::ignore_line;
                     continue;
                 }

                 si >> std::skipws;

                 if (key == "{") {
                     if (block) {
                         std::clog << fname << ": parse error: { nested block are not supported (line " << 
                         more::line_number(si) << ")" << std::endl;
                         return false;
                     }
                     block = true;
                     continue;
                 }

                 if (key == "}") {
                     if (block)
                         break;
                     std::clog << fname << ": parse error: expected `{' before the end-of-block } (line "<< 
                     more::line_number(si) << ")" << std::endl;
                     return false;
                 }

                 // parse Separator ('=')

                 if ( c != Separator ) {
                     si >> c; 
                     if ( c != Separator ) {
                         std::clog << fname << ": parse error: key[" << key << "] missing separator '" 
                         << Separator << "' (line "<< more::line_number(si) << ")" << std::endl;
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

    template <bool S, char Separator, char Comment>
    class parser<more::TM::null, S, Separator, Comment> {};

    template <typename T, bool Strict = false> 
    struct block : public parser<T, Strict, ':', '#'> {}; 

} // namespace kv
} // namespace more

#endif /* _KV_FILE_HH_ */
