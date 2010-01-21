/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef STREAMER_HH
#define STREAMER_HH

#include <iostream>
#include <cstdio>
#include <string>
#include <mtp.hh>

namespace more {

    // construction-on-the-first idiom allows to share a unique sep_index  
    // between different compilation units 
    //

    struct streamer 
    {
        static int sep_index()
        {
            static int index = std::ios_base::xalloc();
            return index;
        }    

        static std::ostream &
        sep(std::ostream &out, const char * sep = NULL)
        {
            out.iword(sep_index()) = reinterpret_cast<long>(sep);
            return out;
        }

    };

    // printable mangling traits
    //

    template <typename T>
    struct sc_mangling_traits 
    { typedef T type; };
    template <>
    struct sc_mangling_traits<char>
    { typedef std::string type; };
    template <>
    struct sc_mangling_traits<unsigned char>
    { typedef std::string type; };

    // type mangling
    //

    template <typename T>
    inline typename std::tr1::add_const< typename sc_mangling_traits<T>::type >::type sc_type_mangling(const T &t)
    { return t; }

    template <>
    inline
    std::tr1::add_const<sc_mangling_traits<char>::type>::type 
    sc_type_mangling<char>(const char &c)
    {
        char buf[8];
        sprintf(buf, (c > 31 && c < 127) ? "%c" : "0x%x", static_cast<unsigned char>(c));
        return buf;
    }
    template <>
    inline
    std::tr1::add_const<sc_mangling_traits<unsigned char>::type>::type
    sc_type_mangling<unsigned char>(const unsigned char &c)
    {
        char buf[8];
        sprintf(buf, (c > 31 && c < 127) ? "%c" : "0x%x", static_cast<unsigned char>(c));
        return buf;
    }

}

namespace std {
    
    // streamer...
    //

    template <typename CharT, typename Traits, typename T>
    inline typename more::mtp::enable_if_c< more::traits::is_container<T>::value && 
    !tr1::is_same<typename std::string,T>::value, 
           std::basic_ostream<CharT,Traits> >::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T &v)
    {
        typename T::const_iterator it = v.begin();
        for(; it != v.end();) {
            out << more::sc_type_mangling(*it); 
            if ( ++it != v.end() && out.iword(more::streamer::sep_index()) ) {
                out << reinterpret_cast<char *>(out.iword(more::streamer::sep_index()));   
            } 
        }
        return out;
    };

    template <typename CharT, typename Traits, typename U, typename V>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, const std::pair<U,V> &r)
    {
        out << '<' << r.first << ':' << r.second << '>';
        return out;
    }
}

#endif /* STREAMER_HH */
