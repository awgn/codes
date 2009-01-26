/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef DUMP_CONTAINER_HH
#define DUMP_CONTAINER_HH

#include <iostream>
#include <string>
#include <mtp.hh>
#include <gcc_version.h>

namespace more {

    // construction-on-the-first idiom allows to share a unique sep_index  
    // between different compilation units 
    //

    struct dump 
    {
        static int sep_index()
        {
            static int index = std::ios_base::xalloc();
            return index;
        }    

        static std::ostream &
        sep(std::ostream &out, char sep = '\0')
        {
            out.iword(dump::sep_index()) = static_cast<int>(sep);
            return out;
        }

    };

    // printable mangling traits
    //

    template <typename T>
    struct dump_mangling_traits 
    { typedef T type; };
    template <>
    struct dump_mangling_traits<char>
    { typedef std::string type; };
    template <>
    struct dump_mangling_traits<unsigned char>
    { typedef std::string type; };

    // type mangling
    //

    template <typename T>
    static inline typename std::tr1::add_const< typename dump_mangling_traits<T>::type >::type  dump_type_mangling(const T &t)
    { return t; }

    template <>
    #if GCC_VERSION < 40300
    static 
    #endif
    inline
    std::tr1::add_const<dump_mangling_traits<char>::type>::type 
    dump_type_mangling<char>(const char &c)
    {
        char buf[8];
        sprintf(buf, (c > 31 && c < 127) ? "%c" : "0x%x", static_cast<unsigned char>(c));
        return buf;
    }
    template <>
    #if GCC_VERSION < 40300
    static 
    #endif
    inline
    std::tr1::add_const<dump_mangling_traits<unsigned char>::type>::type
    dump_type_mangling<unsigned char>(const unsigned char &c)
    {
        char buf[8];
        sprintf(buf, (c > 31 && c < 127) ? "%c" : "0x%x", static_cast<unsigned char>(c));
        return buf;
    }

}

namespace std {
    
    // dumper

    template <typename T>
    typename mtp::enable_if< traits::is_container<T>::value && !tr1::is_same<T, std::string>::value, std::ostream>::type &
    operator<<(std::ostream &out, const T &v)
    {
        typename T::const_iterator it = v.begin();
        for(; it != v.end();) {
            out << more::dump_type_mangling(*it); 
            if ( ++it != v.end() && out.iword(more::dump::sep_index()) ) {
                out << static_cast<char>(out.iword(more::dump::sep_index()));   
            } 
        }
        return out;
    };

    template <typename U, typename V>
    inline std::ostream &
    operator<< (std::ostream &out, const std::pair<U,V> &r)
    {
        out << '<' << r.first << ',' << r.second << '>';
        return out;
    }
}

#endif /* DUMP_CONTAINER_HH */
