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
#include <tr1/type_traits>

#include <typemap.hh>

//////////////////////////////////
//  key-value config file parser 

namespace more {

    namespace kv {

        template <int n>
        struct int2type
        {
            enum { value = n };
        };

        class lnistreambuf : public std::streambuf {

            std::streambuf * _M_in;
            int _M_line;

        public:
            lnistreambuf(std::streambuf *in)
            : _M_in(in),
            _M_line(1)
            {}

            virtual int_type underflow()
            {
                return  _M_in->sgetc();
            }

            virtual int_type uflow()
            {
                int_type c = _M_in->sbumpc();
                if ( c == '\n' )
                    _M_line++;
                return c;
            }

            int line() const
            { return _M_line; }

        };

        static inline
        int lineno(std::istream &in) {
            lnistreambuf * ln = dynamic_cast<lnistreambuf *>(in.rdbuf());
            if (ln) {
                return ln->line();
            }
            return -1;
        }

        static inline
        void skipline(std::istream &si) {
            char c;
            si >> std::noskipws;
            while( si >> c && c != '\n') {
                continue;
            }
            si >> std::skipws;
        }

        template <typename T, bool S> struct block;

        /////////////////////////////////////////////////////////////////////////////////
        // overloaded lex_parse functions must be provided to parse user-defined types
        //      note: specializations do not partecipate to overloading

        template <typename E>
        static inline bool lex_parse(std::istream &in, E &elem)
        { 
            return (in >> elem);
        }        

        template <typename T, bool S>
        static inline bool lex_parse(std::istream &in, block<T,S> &b)
        { 
            return b.parse(in, "block");
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
        template <typename E>
        static inline bool lex_parse(std::istream &in, std::vector<E> &elems)
        {
            E tmp;
            if ( lex_parse(in,tmp) ) {
                elems.push_back(tmp);
                return true;
            }
            return false;
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
                     typedef typename T::type value_type;

                     typedef parser<typename T::next, SMD, SEP, COM> map_type;

                     map_type     _M_map;
                     key_type     _M_key;
                     value_type   _M_value;

                     parser()
                     : _M_map(),
                       _M_key(),
                       _M_value() 
                     {}

                     virtual ~parser()
                     {}

                     //////////////////////////////////////////////////////////////////////////
                     // compile-time get

                     template <typename K>
                     typename std::tr1::add_reference< typename mtp::TM::get<K, T>::type>::type
                     get() 
                     { return __get<K>(int2type< mtp::TM::indexof<K, T>::value >()); }

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
                                              U::key::value() << "] unexpected argument (line " << lineno(in) << ")" << std::endl;
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
                             std::clog << fname << ": parse error: key[" << key << "] unknown (line " << lineno(in) << ")" << std::endl;
                             return false;
                         }

                         // non-strict mode: skip this line
                         //

                         skipline(in);
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

                             // std::cout << "KEY ["  << key << "]\n";

                             // skip comments/empty lines
                             //

                             if (key.empty() || key[0] == COM ) {
                                 skipline(si);
                                 continue;
                             }

                             si >> std::skipws;

                             if (key == "{") {
                                 if (block) {
                                     std::clog << fname << ": parse error: { nested block are not supported (line " << lineno(si) << ")" << std::endl;
                                     return false;
                                 }
                                 block = true;
                                 continue;
                             }

                             if (key == "}") {
                                 if (block)
                                     break;
                                 std::clog << fname << ": parse error: expected `{' before the end-of-block } (line "<< lineno(si) << ")" << std::endl;
                                 return false;
                             }

                             // parse SEPARATOR ('=')
                             //

                             if ( c != SEP ) {
                                 si >> c; 
                                 if ( c != SEP ) {
                                     std::clog << fname << ": parse error: key[" << key << "] missing separator '" 
                                     << SEP << "' (line "<< lineno(si) << ")" << std::endl;
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
