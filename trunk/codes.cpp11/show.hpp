/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SHOW_HPP_
#define _SHOW_HPP_

#include <type_traits.hpp>  // more!

#include <type_traits>
#include <array>
#include <tuple>
#include <chrono>
#include <memory>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <sstream>

#include <cxxabi.h>


inline namespace more_show {

    // forward declarations:
    //

    inline std::string 
    show(const char *v, const char *n = nullptr);

    inline std::string 
    show(std::string const &s, const char *n = nullptr);

    template <typename T> 
    inline 
    typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type 
    show(T const &value, const char * n = nullptr);

    template <typename T>
    inline  
    typename std::enable_if<std::is_pointer<T>::value, std::string>::type 
    show(T const &p, const char *n = nullptr);

    template <typename U, typename V>
    inline std::string
    show(std::pair<U,V> const &r, const char * n = nullptr);

    template <typename T, std::size_t N>
    inline std::string
    show(std::array<T,N> const &a, const char * n = nullptr);

    template <typename ...Ts>
    inline std::string
    show(std::tuple<Ts...> const &t, const char * n = nullptr);

    template <typename Rep, typename Period>
    inline std::string
    show(std::chrono::duration<Rep, Period> const &dur, const char *n = nullptr);

    template <typename Clock, typename Dur>
    inline std::string
    show(std::chrono::time_point<Clock, Dur> const &r, const char *n = nullptr);

    template <typename T>
    inline typename std::enable_if<
    (!std::is_pointer<T>::value) && (
        (more::traits::is_container<T>::value && !std::is_same<typename std::string,T>::value) ||
        (std::rank<T>::value > 0 && !std::is_same<char, typename std::remove_cv<typename std::remove_all_extents<T>::type>::type>::value)),
    std::string>::type 
    show(const T &v, const char * n = nullptr);

    
    namespace show_helper
    {
        // utilities 
        //

        inline std::string
        demangle(const char *name)
        {
            int status;
            std::unique_ptr<char, void(*)(void *)> ret(abi::__cxa_demangle(name,0,0, &status), ::free);
            if (status < 0) {
                return std::string(1,'?');
            }
            return std::string(ret.get());
        }

        template <typename T>
        inline std::string 
        header(const char *n)
        {
            return n == nullptr ? std::string() : 
            *n == '\0' ? demangle(typeid(T).name()) + ' ' :
            std::string(n) + ' ';
        }

        // show_on policy 
        //

        template <typename T, int N>
        struct show_on
        {
            static inline
            void apply(std::string &out, const T &tupl, const char *n)
            {
                out += show(std::get< std::tuple_size<T>::value - N>(tupl), nullptr) + ' ';
                show_on<T,N-1>::apply(out,tupl, n);
            }
        }; 
        template <typename T>
        struct show_on<T, 0>
        {
            static inline
            void apply(std::string&, const T &, const char *)
            {}
        };

        template <typename T>
        struct duration_traits;
        template <> struct duration_traits<std::chrono::nanoseconds>  { static constexpr const char *str = "_ns"; };
        template <> struct duration_traits<std::chrono::microseconds> { static constexpr const char *str = "_us"; };
        template <> struct duration_traits<std::chrono::milliseconds> { static constexpr const char *str = "_ms"; };
        template <> struct duration_traits<std::chrono::seconds>      { static constexpr const char *str = "_s"; };
        template <> struct duration_traits<std::chrono::minutes>      { static constexpr const char *str = "_m"; };
        template <> struct duration_traits<std::chrono::hours>        { static constexpr const char *str = "_h"; };

    } // namespace show_helper

    ///////////////////////////////////////
    // show for const char *
    //

    inline std::string
    show(const char *v, const char *n)
    {
        return show_helper::header<const char *>(n) + '"' + std::string(v) + '"';
    }

    ///////////////////////////////////////
    // show for std::string
    //

    inline std::string
    show(std::string const &s, const char *n)
    {
        return show_helper::header<std::string>(n) + '"' + s + '"';
    }

    ///////////////////////////////////////
    // show for arithmetic types..
    //

    template <typename T>
    inline typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type
    show(T const &value, const char * n)
    {
        return show_helper::header<T>(n) + std::to_string(value);
    }

    ///////////////////////////////////////
    // show for pointers *
    //

    template <typename T> 
    inline typename std::enable_if<std::is_pointer<T>::value, std::string>::type
    show(T const &p, const char *n)
    {
        std::ostringstream o;
        o << static_cast<void *>(p);
        return show_helper::header<T>(n) + o.str();
    }

    //////////////////////////
    // show for pair...

    template <typename U, typename V>
    inline std::string
    show(const std::pair<U,V> &r, const char * n)
    {
        return show_helper::header<std::pair<U,V>>(n) + 
        '(' + show(r.first) + 
        ',' + show(r.second) + ')';
    }

    ///////////////////////////
    // show for array...

    template <typename T, std::size_t N>
    inline std::string
    show(std::array<T,N> const &a, const char * n)
    {
        std::string out("[ ");
        show_helper::show_on<std::array<T,N>, N>::apply(out,a, n);
        return show_helper::header<std::array<T,N>>(n) + out + ']';
    }

    ////////////////////////////////////////////////////////
    // show for tuple... 

    template <typename ...Ts>
    inline std::string
    show(std::tuple<Ts...> const &t, const char * n)
    {
        std::string out("{ ");
        show_helper::show_on<std::tuple<Ts...>, sizeof...(Ts)>::apply(out,t,n);
        return show_helper::header<std::tuple<Ts...>>(n) + out + '}';
    }                                              

    ////////////////////////////////////////////////////////
    // show for chrono types... 

    template <typename Rep, typename Period>
    inline std::string
    show(std::chrono::duration<Rep, Period> const &dur, const char *n)
    {
        std::string out(std::to_string(dur.count()));
        return show_helper::header<std::chrono::duration<Rep,Period>>(n) + out + std::string(show_helper::duration_traits<std::chrono::duration<Rep,Period>>::str);
    }

    template <typename Clock, typename Dur>
    inline std::string
    show(std::chrono::time_point<Clock, Dur> const &r, const char *n)
    {    
        return show_helper::header<std::chrono::time_point<Clock,Dur>>(n) + show(r.time_since_epoch());
    }

    ///////////////////////////////////////
    // show for generic containers...
    //

    template <typename T>
    inline typename std::enable_if<
    (!std::is_pointer<T>::value) && (
        (more::traits::is_container<T>::value && !std::is_same<typename std::string,T>::value) ||
        (std::rank<T>::value > 0 && !std::is_same<char, typename std::remove_cv<typename std::remove_all_extents<T>::type>::type>::value)),
    std::string>::type 
    show(const T &v, const char * n)
    {
        std::string s("{ ");
        for(auto & e : v)
        {
            s += show(e) + ' ';
        }
        return show_helper::header<T>(n) + s + '}';
    };

} // namespace more_show


#endif /* _SHOW_HPP_ */
