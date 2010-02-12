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

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

#include <tr1/type_traits>

#include <typemap.hh>           // more
#include <iomanip.hh>           // more
#include <lnistreambuf.hh>      // more

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
    template <typename E, template <typename _Tp, typename Alloc = std::allocator<_Tp> > class C >
    inline bool lex_parse(std::istream &in, C<E> &elems)
    {
        E tmp;
        if ( lex_parse(in,tmp) ) {
            elems.push_back(tmp);
            return true;
        }
        return false;
    }
    // std::map that implements associative container 
    //
    template <typename K, typename V>
    inline bool lex_parse(std::istream &in, std::map<K,V> &elems)
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
        }
        else { // simple string
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
             bool SMD  = false /* strict mode */, 
             char SEP  = '='  /* separator */, 
             char COM  = '#' /* comment */ >
     struct parser
     {
     public:
         typedef typename T::key key_type;
         typedef typename T::value value_type;

         typedef parser<typename T::next, SMD, SEP, COM> map_type;

         map_type     _M_map;
         key_type     _M_key;
         value_type   _M_value;

         parser()
         : _M_map(),
         _M_key(),
         _M_value(get_default<key_type, value_type, key_type::has_default>::value()) 
         {}

         virtual ~parser()
         {}

         //////////////////////////////////////////////////////////////////////////
         // compile-time get

         template <typename K>
         typename std::tr1::add_reference< typename mtp::TM::get<K, T>::type>::type
         get() 
         { return __get<K>(int2type< mtp::TM::index_of<K, T>::value >()); }

         template <typename K, int n>
         typename std::tr1::add_reference<typename mtp::TM::get<K, T>::type>::type
         __get(int2type<n>) 
         { return _M_map.__get<K>(int2type<n-1>()); }

         template <typename K>
         typename std::tr1::add_reference<value_type>::type
         __get(int2type<0>) 
         { return _M_value; } 

     protected:
         //////////////////////////////////////////////////////////////////////////
         // run-time parser 

         bool parse_key_value(std::istream &in, const std::string &fname, const std::string &key)
         { return __parse(in, fname, key, *this); }

         template <typename U, bool S >
         static bool __parse(std::istream &in, const std::string &fname, const std::string &key, parser<U,S,SEP,COM> &that)
         {
             if (key == U::key::value()) {
                 if (!lex_parse(in,that._M_value) || in.fail() ) {
                     std::clog << fname << ": parse error: key[" << 
                     U::key::value() << "] unexpected argument (line " << 
                     more::line_number(in) << ")" << std::endl;
                     return false;
                 }
                 return true;
             }
             return __parse(in, fname, key, that._M_map);
         }
         template <bool S>
         static bool __parse(std::istream &in, const std::string &fname, const std::string &key, parser<mtp::TM::null,S,SEP,COM> &)
         {
             // unknown key-value!
             //

             if (S) {   // strict mode: dump-error 
                 std::clog << fname << ": parse error: key[" << key << "] unknown (line " << 
                 more::line_number(in) << ")" << std::endl;
                 return false;
             }

             // non-strict mode: skip this line
             //
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
                 //

                 char c('\0');
                 while ( si >> c && !isspace(c) && c != SEP ) {
                     key.push_back(c);
                 }

                 // skip comments/empty lines
                 //

                 if (key.empty() || key[0] == COM ) {
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

                 // parse SEPARATOR ('=')
                 //

                 if ( c != SEP ) {
                     si >> c; 
                     if ( c != SEP ) {
                         std::clog << fname << ": parse error: key[" << key << "] missing separator '" 
                         << SEP << "' (line "<< more::line_number(si) << ")" << std::endl;
                         return false;
                     }
                 }

                 // parse value... 
                 //

                 if ( !parse_key_value(si, fname, key) ) 
                     return false;
             }
             // std::cout << "EOF\n";
             return true;
         }
     };

    template <bool S, char SEP, char COM>
    class parser<mtp::TM::null, S, SEP, COM> {};

    template <typename T, bool SMD = false> 
    struct block : public parser<T, SMD, ':', '#'> {}; 

} // namespace kv
} // namespace more

#endif /* _KV_FILE_HH_ */
