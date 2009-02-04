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

        /////////////////////////////////////////////////////////////////////////////////
        // overloaded lex_parse functions must be provided to parse user-defined types

        template <typename E>
        static bool lex_parse(std::istream &in, E &elem)
        { 
            return (in >> elem);
        }
        template <>
        bool lex_parse<bool>(std::istream &in, bool &elem)
        {
            in >> std::noboolalpha;
            if (!(in >> elem)) {
                in.clear();
                return (in >> std::boolalpha >> elem);
            }
            return true;
        }
        template <>
        bool lex_parse<std::string>(std::istream &in, std::string &elem)
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
                    std::clog << "parse: error at string '" << elem << ": missing quotation.\n";
                    return false;
                }
            }
            else {              // simple string
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
        static bool lex_parse(std::istream &in, std::vector<E> &elems)
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

        template <typename T, bool SMD  = false /* strict mode */, 
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

        public:
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

            bool parse_keyvalue(std::istream &in, const std::string &fname, const std::string &key)
            { return __parse(in, fname, key, *this); }

            template <typename U, bool S >
            static bool __parse(std::istream &in, const std::string &fname, const std::string &key, parser<U,S,SEP,COM> &that)
            {
                if (key == U::key::value()) {
                    if (!lex_parse(in,that._M_value) || in.fail() ) {
                        std::clog << fname << ": parse error: key[" << U::key::value() << "] unexpected argument";
                        return false;
                    }
                    return true;
                }

                return __parse(in, fname, key, that._M_map);
            }
            template <bool S>
            static bool __parse(std::istream &in, const std::string &fname, const std::string &k, parser<mtp::TM::null,S,SEP,COM> &)
            {
                if (S)
                    std::clog << fname << ": parse error: key[" << k << "] unknown";
                return false;
            }

        public:

            bool parse(const std::string &fname)
            {
                std::ifstream sc(fname.c_str());
                if (!sc) {
                    std::clog << fname << ": parse error: no such file\n";
                    return false;
                }

                return parse(sc, fname);
            }

            bool parse(std::istream &sc, const std::string &fname = "unnamed")
            {
                std::string line;
                for(int n = 1; std::getline(sc, line); n++) {

                    std::stringstream sline(line);
                    std::string key;

                    sline.unsetf(std::ios::dec);
                    sline.unsetf(std::ios::hex);
                    sline.unsetf(std::ios::oct);

                    // parse key...
                    sline >> std::noskipws;
                    sline >> std::ws;

                    char c('\0');
                    while ( sline >> c && c != SEP && !isspace(c)) {
                        key.push_back(c);
                    }

                    sline >> std::skipws;
                    // std::cout << "KEY{"  << key << "}\n";

                    // skip comments/empty lines
                    if (key.empty() || key[0] == COM )
                        continue;

                    // parse '='
                    if ( c != SEP ) {
                        char eq('\0'); sline >> eq;
                        // std::cout << "EQ{" << eq << "}\n";
                        if ( eq != SEP ) {
                            std::clog << fname << ": parse error: key[" << key << "] missing separator '" << SEP << "' (line "<< n << ")\n";
                            return false;
                        }
                    }

                    sline >> std::ws;

                    // parse value... 
                    if ( !parse_keyvalue(sline, fname, key) ) {
                        if (SMD) {
                            std::clog << " (line " << n << ")\n";
                            return false;
                        } else
                            continue;
                    }

                    std::string garbage; sline >> garbage;

                    // std::cout << "GARBAGE{" << garbage << "}\n";
                    if (!garbage.empty() && garbage[0] != COM ) {
                        std::clog << fname << ": parse error: key[" << key << "] trailing garbage (line " << n << ")\n";
                        return false;
                    }
                }   

                return true;
            }
        };

        template <bool S, char SEP, char COM>
        class parser<mtp::TM::null, S, SEP, COM> {};


    } // namespace kv

}// namespace more

#endif /* _KV_FILE_HH_ */
