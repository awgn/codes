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
#include <show.hpp>         // more!

#include <type_traits>
#include <array>
#include <tuple>
#include <chrono>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>

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
        return out << show (v);
    }

    //////////////////////////
    // operator<< for pair...

    template <typename CharT, typename Traits, typename U, typename V>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, std::pair<U,V> const &p)
    {
        return out << show (p);
    }

    ///////////////////////////
    // operator<< for array...

    template <typename CharT, typename Traits, typename T, std::size_t N>
    inline std::basic_ostream<CharT,Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, std::array<T,N> const &a)
    {
        return out << show (a);
    }

    //////////////////////////
    // operator<< for tuple...

    template <typename CharT, typename Traits, typename ...Ts>
    inline std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, std::tuple<Ts...> const &t)
    {
        return out << show (t);
    }

    /////////////////////////////////
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
        return out << show (r);
    }

    template <typename CharT, typename Traits, typename Clock, typename Dur>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, std::chrono::time_point<Clock, Dur> const &r)
    {
        return out << show (r);
    }

} // namespace std


#endif /* _STREAMER_HPP_ */
