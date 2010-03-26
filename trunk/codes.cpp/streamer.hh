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

#include <mtp.hh>           // more!
#include <type_traits.hh>   // more!

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <tr1/type_traits>
#include <tr1/array>
#include <tr1/tuple>
namespace std { using namespace std::tr1; }
#else
#include <type_traits>
#include <array>
#include <tuple>
#endif

#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <string>

namespace more {

    namespace streamer {

        template <typename T>
        static inline const T &
        print(const T &elem)
        {
            return elem;
        }
        
        static inline std::string
        print(char c)
        {
            if ( c > 31 && c < 127 )
                return std::string(1,c);
            std::stringstream s;
            s << "0x" << std::hex << static_cast<int>(c);
            return s.str();
        }

        // construction-on-the-first idiom allows to share a unique sep_index  
        // between different compilation units 
        //

        static int sep_index()
        {
            static int index = std::ios_base::xalloc();
            return index;
        }    

        template <typename CharT, typename Traits>
        std::basic_ostream<CharT, Traits> &
        sep(std::basic_ostream<CharT, Traits> &out, const char * sep = NULL)
        {
            out.iword(sep_index()) = reinterpret_cast<long>(sep);
            return out;
        }    

        template <typename CharT, typename Traits, typename T>
        struct dumper : public std::unary_function<typename T::value_type, void>
        {
            dumper(std::basic_ostream<CharT, Traits> &out)
            : _M_out(out), _M_sep(reinterpret_cast<char *>(out.iword(streamer::sep_index())))
            {}

            void operator()(const typename T::value_type & elem) const
            {
                _M_out << more::streamer::print(elem);
                if (_M_sep)
                    _M_out << _M_sep;
            }

        private:
            std::basic_ostream<CharT, Traits> & _M_out;
            char * _M_sep;
        };

        namespace tuplarr {

            // printon policy 

            template <typename CharT, typename Traits, typename T, int N>
            struct printon
            {
                static void apply(std::basic_ostream<CharT,Traits> &out, const T &tupl)
                {
                    out << std::get< std::tuple_size<T>::value - N>(tupl) << ' ';
                    printon<CharT, Traits, T,N-1>::apply(out,tupl);
                }

            };
            template <typename CharT, typename Traits, typename T>
            struct printon<CharT, Traits, T,0>
            {
                static void apply(std::basic_ostream<CharT, Traits> &out, const T &)
                {}
            };
        }
    }
}

namespace std {

    ///////////////////////////////////////
    // operator<< for generic containers...
    //

    template <typename CharT, typename Traits, typename T>
    inline typename more::mtp::enable_if_c< more::traits::is_container<T>::value && 
    !is_same<typename std::string,T>::value, 
    std::basic_ostream<CharT,Traits> >::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T &v)
    {
        std::for_each(v.begin(), v.end(), more::streamer::dumper<CharT, Traits, T>(out));
        return out;
    };

    //////////////////////////
    // operator<< for pair...

    template <typename CharT, typename Traits, typename U, typename V>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, const std::pair<U,V> &r)
    {
        return out << '<' << r.first << ':' << r.second << '>';
    }

#ifndef __GXX_EXPERIMENTAL_CXX0X__
    namespace tr1 
    {
#endif
        ///////////////////////////
        // operator<< for array...

        template <typename CharT, typename Traits, typename T, std::size_t N>
        std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT,Traits> &out, const std::array<T,N> & rhs)
        {
            out << "[ ";
            more::streamer::tuplarr::printon<CharT, Traits, std::array<T,N>, N>::apply(out,rhs);
            return out << "]";
        }

        ////////////////////////////////////////////////////////
        // operator<< for tuple: (enabled if T is a tuple<>)... 

        template <typename CharT, typename Traits, typename T>
        typename more::mtp::enable_if< more::traits::is_tuple<T>, std::basic_ostream<CharT,Traits> >::type &
        operator<<(std::basic_ostream<CharT,Traits> &out, const T & rhs)
        {
            out << "< ";
            more::streamer::tuplarr::printon<CharT, Traits, T, std::tuple_size<T>::value>::apply(out,rhs);
            return out << ">";
        }

#ifndef __GXX_EXPERIMENTAL_CXX0X__
    }   // namespace tr1
#endif

} // namespace std

#endif /* STREAMER_HH */
