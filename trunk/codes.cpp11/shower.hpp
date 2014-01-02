/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#pragma once

#include <show.hpp>
#include <macro.hpp>

#include <tuple>
#include <string>

#define SHOWER_PAIR_1(a, x)        std::make_pair(std::string(#a), &a::x) 
#define SHOWER_PAIR_2(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_1(a,  __VA_ARGS__)
#define SHOWER_PAIR_3(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_2(a,  __VA_ARGS__)
#define SHOWER_PAIR_4(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_3(a,  __VA_ARGS__)
#define SHOWER_PAIR_5(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_4(a,  __VA_ARGS__)
#define SHOWER_PAIR_6(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_5(a,  __VA_ARGS__)
#define SHOWER_PAIR_7(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_6(a,  __VA_ARGS__)
#define SHOWER_PAIR_8(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_7(a,  __VA_ARGS__)
#define SHOWER_PAIR_9(a, x, ...)   std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_8(a,  __VA_ARGS__)
#define SHOWER_PAIR_10(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_9(a,  __VA_ARGS__)
#define SHOWER_PAIR_11(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_10(a, __VA_ARGS__)
#define SHOWER_PAIR_12(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_11(a, __VA_ARGS__)
#define SHOWER_PAIR_13(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_12(a, __VA_ARGS__)
#define SHOWER_PAIR_14(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_13(a, __VA_ARGS__)
#define SHOWER_PAIR_15(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_14(a, __VA_ARGS__)
#define SHOWER_PAIR_16(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_15(a, __VA_ARGS__)
#define SHOWER_PAIR_17(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_16(a, __VA_ARGS__)
#define SHOWER_PAIR_18(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_17(a, __VA_ARGS__)
#define SHOWER_PAIR_19(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_18(a, __VA_ARGS__)
#define SHOWER_PAIR_20(a, x, ...)  std::make_pair(std::string(#a), &a::x) , SHOWER_PAIR_19(a, __VA_ARGS__)

#define SHOWER_PAIR(a, ...)  XPASTE(SHOWER_PAIR_, PP_NARG(__VA_ARGS__))(a, __VA_ARGS__)


#define MAKE_SHOW(type, ...) more::make_shower<type>(SHOWER_PAIR(type, __VA_ARGS__))
 

namespace more {

    namespace details
    {
         inline std::string
        demangle(const char * name)
        {
            int status;
            std::unique_ptr<char, void(*)(void *)> ret(abi::__cxa_demangle(name,0,0, &status), ::free);
            if (status < 0) {
                return std::string(1,'?');
            }
            return std::string(ret.get());
        }

       template <typename T, typename Tp, int N>
        struct shower_on
        {
            static inline
            void apply(std::string &out, const T &tupl, const Tp &value)
            {
                auto p = std::get< std::tuple_size<T>::value - N>(tupl);

                out += p.first + " = " +  show (value.*p.second);
                if (N > 1) 
                    out += ", ";
                shower_on<T, Tp, N-1>::apply(out,tupl, value);
            }
        }; 
        template <typename T, typename Tp>
        struct shower_on<T, Tp, 0>
        {
            static inline
            void apply(std::string&, const T &, const Tp &)
            {}
        };
    }

    template <typename Tp, typename ...Ps>
    struct shower
    {
        template <typename ...Ts>
        shower(Ts && ...args)
        : data_(std::forward<Ts>(args)...)
        {}

        std::string
        operator()(Tp const &value)
        {
            auto out = details::demangle(typeid(Tp).name()) + "{";
        
            details::shower_on<std::tuple<Ps...>, Tp, sizeof...(Ps)>::apply(out, data_, value); 

            return out + "}"; 
        }

        std::tuple<Ps...> data_;
    };
    
    template <typename Tp, typename ...Ts>
    more::shower<Tp, Ts...>
    make_shower(Ts && ... args)
    {
        return more::shower<Tp, Ts...>(std::forward<Ts>(args)...);
    }

} // namespace more
