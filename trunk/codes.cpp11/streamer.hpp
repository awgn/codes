/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _STREAMER_HPP_
#define _STREAMER_HPP_

#include <type_traits.hpp>  // more!

#include <type_traits>
#include <array>
#include <tuple>
#include <chrono>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>

namespace more { namespace streamer {

        // printon policy
        //

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
            static void apply(std::basic_ostream<CharT, Traits> &, const T &)
            {}
        };

        template <typename T>
        struct _duration_traits;
            template <> struct _duration_traits<std::chrono::nanoseconds>  { static constexpr const char *str = "_ns"; };
            template <> struct _duration_traits<std::chrono::microseconds> { static constexpr const char *str = "_us"; };
            template <> struct _duration_traits<std::chrono::milliseconds> { static constexpr const char *str = "_ms"; };
            template <> struct _duration_traits<std::chrono::seconds>      { static constexpr const char *str = "_s"; };
            template <> struct _duration_traits<std::chrono::minutes>      { static constexpr const char *str = "_m"; };
            template <> struct _duration_traits<std::chrono::hours>        { static constexpr const char *str = "_h"; };

} // namespace streamer
} // namespace more


namespace std {

    ///////////////////////////////////////
    // operator<< for generic containers...
    //

    template <typename CharT, typename Traits, typename T>
    inline typename std::enable_if<
        (more::traits::is_container<T>::value && !is_same<typename std::string,T>::value) ||
        (rank<T>::value > 0 && !is_same<char, typename remove_cv<typename remove_all_extents<T>::type>::type>::value),
        std::basic_ostream<CharT,Traits>>::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T &v)
    {
        out << "{ ";
        for(auto const & e : v)
        {
            out << e << ' ';
        }
        return out << '}';
    }

    //////////////////////////
    // operator<< for pair...

    template <typename CharT, typename Traits, typename U, typename V>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, const std::pair<U,V> &r)
    {
        return out << '(' << r.first << ',' << r.second << ')';
    }

    ///////////////////////////
    // operator<< for array...

    template <typename CharT, typename Traits, typename T, std::size_t N>
    std::basic_ostream<CharT,Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const std::array<T,N> &rhs)
    {
        out << "[ ";
        more::streamer::printon<CharT, Traits, std::array<T,N>, N>::apply(out,rhs);
        return out << ']';
    }

    ///////////////////////////////////////////////////////
    // operator<< for tuple: (enabled if T is a tuple<>)...

    template <typename CharT, typename Traits, typename T>
    typename std::enable_if< more::traits::is_tuple<T>::value, std::basic_ostream<CharT,Traits> >::type &
    operator<<(std::basic_ostream<CharT,Traits> &out, const T &rhs)
    {
        out << "( ";
        more::streamer::printon<CharT, Traits, T, std::tuple_size<T>::value>::apply(out,rhs);
        return out << ')';
    }

    ////////////////////////////////////////////////////////
    // operator<< for chrono types...

    template <typename CharT, typename Traits, typename Tp>
    typename std::enable_if< std::is_same<Tp, std::chrono::nanoseconds>::value  ||
                             std::is_same<Tp, std::chrono::microseconds>::value ||
                             std::is_same<Tp, std::chrono::milliseconds>::value ||
                             std::is_same<Tp, std::chrono::seconds>::value      ||
                             std::is_same<Tp, std::chrono::minutes>::value      ||
                             std::is_same<Tp, std::chrono::hours>::value,
             std::basic_ostream<CharT, Traits>>::type &
    operator<< (std::basic_ostream<CharT, Traits> &out, const Tp &r)
    {
        return out << r.count() << more::streamer::_duration_traits<Tp>::str;
    }

    template <typename CharT, typename Traits, typename Clock, typename Dur>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, std::chrono::time_point<Clock, Dur> const &r)
    {
        return out << r.time_since_epoch();
    }

} // namespace std


#endif /* _STREAMER_HPP_ */
