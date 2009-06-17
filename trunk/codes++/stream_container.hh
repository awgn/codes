/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef STREAM_CONTAINER_HH
#define STREAM_CONTAINER_HH

#include <iostream>
#include <string>
#include <mtp.hh>

namespace more {

    // construction-on-the-first idiom allows to share a unique sep_index  
    // between different compilation units 
    //

    struct stream_container 
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
    static inline typename std::tr1::add_const< typename sc_mangling_traits<T>::type >::type  sc_type_mangling(const T &t)
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
    
    // dumper

    template <typename T>
    typename mtp::enable_if< more::traits::is_container<T>::value && !tr1::is_same<T, std::string>::value, std::ostream>::type &
    operator<<(std::ostream &out, const T &v)
    {
        typename T::const_iterator it = v.begin();
        for(; it != v.end();) {
            out << more::sc_type_mangling(*it); 
            if ( ++it != v.end() && out.iword(more::stream_container::sep_index()) ) {
                out << reinterpret_cast<char *>(out.iword(more::stream_container::sep_index()));   
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

#endif /* STREAM_CONTAINER_HH */
