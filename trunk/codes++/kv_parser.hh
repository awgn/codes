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

/////////////////////////////////////
//  kv: key-value config file parser 

namespace more {

    template <int n>
    struct kv_int2type
    {
        enum { value = n };
    };

    ////////////////////////////////////////////////////////////////////////////////////
    // overloaded kv_parse_elem functions must be provided to parse user-defined types

    template <typename E>
    static bool kv_parse_elem(std::istream &in, E &elem)
    { 
       return (in >> elem);
    }
    static bool kv_parse_elem(std::istream &in, bool &elem)
    {
        in >> std::noboolalpha;
        if (!(in >> elem)) {
            in.clear();
            return (in >> std::boolalpha >> elem);
        }
        return true;
    }
    static bool kv_parse_elem(std::istream &in, std::string &elem)
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
    static bool kv_parse_elem(std::istream &in, std::vector<E> &elems)
    {
        E tmp;
        if ( kv_parse_elem(in,tmp) ) {
            elems.push_back(tmp);
            return true;
        }
        return false;
    }

    template <typename T>
    struct kv_parser
    {
    public:
        typedef typename T::key key_type;
        typedef typename T::type value_type;
        typedef kv_parser<typename T::next> map_type;

        key_type     _M_key;
        value_type   _M_value;
        map_type     _M_map;

        kv_parser()
        : _M_key(), _M_value(), _M_map()
        {}

        virtual ~kv_parser()
        {}

    public:
        //////////////////////////////////////////////////////////////////////////
        // compile-time get

        template <typename K>
        typename std::tr1::add_reference< typename mtp::TM::get<K, T>::type>::type
        get() 
        { return __get<K>(kv_int2type< mtp::TM::indexof<K, T>::value >()); }

        template <typename K, int n>
        typename std::tr1::add_reference<typename mtp::TM::get<K, T>::type>::type
        __get(kv_int2type<n>) 
        { return _M_map.__get<K>(kv_int2type<n-1>()); }

        template <typename K>
        typename std::tr1::add_reference<value_type>::type
        __get(kv_int2type<0>) 
        { return _M_value; } 
       
    protected:
        //////////////////////////////////////////////////////////////////////////
        // run-time parser 

        bool _parse(std::istream &in, const std::string &file, const std::string &key, bool strict)
        { return __parse(in, file, key, strict, *this); }

        template <typename U>
        static bool __parse(std::istream &in, const std::string &file, const std::string &key, bool strict, kv_parser<U> &m)
        {
            if (key == U::key::value()) {
                if (!kv_parse_elem(in,m._M_value) || in.fail() ) {
                    std::clog << file << ": parse error: key[" << U::key::value() << "] unexpected argument";
                    return false;
                }
                return true;
            }
            else return __parse(in, file, key, strict, m._M_map);
        }
        static bool __parse(std::istream &in, const std::string &file, const std::string &k, bool strict, kv_parser<mtp::TM::null> &)
        {
            if (strict)
                std::clog << file << ": parse error: key[" << k << "] unknown";
            return false;
        }
       
    public:

        bool parse(const std::string &file, bool strict = true)
        {
            std::ifstream sc(file.c_str());
            if (!sc) {
                std::clog << file << ": parse error: no such file\n";
                return false;
            }

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
                while ( sline >> c && c != '=' && !isspace(c)) {
                    key.push_back(c);
                }

                sline >> std::skipws;
                // std::cout << "KEY{"  << key << "}\n";

                // skip comments/empty lines
                if (key.empty() || key[0] == '#')
                    continue;

                // parse '='
                if ( c != '=') {
                    char eq('\0'); sline >> eq;
                    // std::cout << "EQ{" << eq << "}\n";
                    if ( eq != '=' ) {
                        std::clog << file << ": parse error: key[" << key << "] missing '=' (line "<< n << ")\n";
                        return false;
                    }
                }

                sline >> std::ws;

                // parse value... 
                if ( !_parse(sline,file, key, strict) ) {
                    if (strict) {
                        std::clog << " (line " << n << ")\n";
                        return false;
                    } else
                        continue;
                }

                std::string garbage; sline >> garbage;

                // std::cout << "GARBAGE{" << garbage << "}\n";
                if (!garbage.empty() && garbage[0] != '#') {
                    std::clog << file << ": parse error: key[" << key << "] trailing garbage (line " << n << ")\n";
                    return false;
                }
            }   

            return true;
        }
    };

    template <>
    class kv_parser<mtp::TM::null> {};
}

#endif /* _KV_FILE_HH_ */
