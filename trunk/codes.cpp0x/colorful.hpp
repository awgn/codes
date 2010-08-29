/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _COLORFUL_HPP_
#define _COLORFUL_HPP_ 

#include <typelist.hpp>
#include <ostream>

namespace more { 

    namespace ecma
    {
        template <int n> struct integral_value { enum { attribute_value = n }; };

        typedef integral_value<0> reset;
        typedef integral_value<1> bold;
        typedef integral_value<2> half_bright;
        typedef integral_value<4> underline;
        typedef integral_value<5> blink;
        typedef integral_value<7> reverse;
        typedef integral_value<8> hidden;
    
        namespace fg 
        {
            typedef integral_value<30> black; 
            typedef integral_value<31> red; 
            typedef integral_value<32> green; 
            typedef integral_value<33> yellow; 
            typedef integral_value<34> blue; 
            typedef integral_value<35> magenta; 
            typedef integral_value<36> cyan; 
            typedef integral_value<37> light_grey; 
        }

        namespace bg
        {
            typedef integral_value<40> black; 
            typedef integral_value<41> red; 
            typedef integral_value<42> green; 
            typedef integral_value<43> yellow; 
            typedef integral_value<44> blue; 
            typedef integral_value<45> magenta; 
            typedef integral_value<46> cyan; 
            typedef integral_value<47> light_grey; 
        }
    };

    template <typename T>
    struct colorful 
    {
        typedef T value_type;
    };


    template <typename T>
    struct ecma_parameter
    {
        typedef T value_type; 
    };

    
    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, ecma_parameter<more::TL::null>)
    {
        return out;
    }

    template <typename CharT, typename Traits, typename T>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, ecma_parameter<T>)
    {
        return out << ";" << T::head::attribute_value << ecma_parameter<typename T::tail>();
    }

    template <typename CharT, typename Traits, typename T>
    inline std::basic_ostream<CharT, Traits> & 
    operator<<(std::basic_ostream<CharT, Traits> &out, colorful<T>)
    {
        return out << "\E[" << ecma_parameter<T>() << "m";
    }

} // namespace more

 
#endif /* _MORE_COLORFUL_HPP_ */
