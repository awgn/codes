/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TRAITS_INFO_HH_
#define _TRAITS_INFO_HH_ 

#include <tr1/type_traits>
#include <tr1/memory>

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <stdexcept>

#include <cxxabi.h>

#ifdef USE_COLORS
#include <colorful.hh>
#include <typelist.hh>
#endif

namespace more { 

    namespace traits_info_helper
    {
#ifdef USE_COLORS
        template <bool B>
        struct color_trait 
        {
            typedef more::colorful< TYPELIST(ecma::fg_blue) > type;
        };

        template <>
        struct color_trait<false>
        {
            typedef more::colorful< TYPELIST(ecma::fg_red) > type;
        };
#endif
    }

#ifdef USE_COLORS
#define __dump_trait_type(x,y)   std::setw(42) << "std::tr1::" #x "<" #y ">: " << \
            typename traits_info_helper::color_trait< std::tr1::x<y>::value >::type() << \
            std::tr1::x<y>::value << \
            "_type" << more::colorful< TYPELIST(ecma::reset) > ()  
#else
#define __dump_trait_type(x,y)   std::setw(42) << "std::tr1::" #x "<" #y ">: " << std::tr1::x<y>::value << "_type"  
#endif

#define __dump_trait_value(x,y)  std::setw(42) << "std::tr1::" #x "<" #y ">: " << std::tr1::x<y>::value  

    template <typename T>
    struct traits_info
    {
        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT,Traits> &out, const traits_info<T> &)
        {
            out << std::boolalpha <<
            __dump_trait_type(is_void,T) << std::endl <<
            __dump_trait_type(is_integral,T) << std::endl << 
            __dump_trait_type(is_floating_point,T) << std::endl << 
            __dump_trait_type(is_array,T) << std::endl << 
            __dump_trait_type(is_pointer,T) << std::endl << 
            __dump_trait_type(is_reference,T) << std::endl << 
            __dump_trait_type(is_member_object_pointer,T) << std::endl << 
            __dump_trait_type(is_member_function_pointer,T) << std::endl << 
            __dump_trait_type(is_enum,T) << std::endl << 
            __dump_trait_type(is_union,T) << std::endl << 
            __dump_trait_type(is_class,T) << std::endl << 
            __dump_trait_type(is_function,T) << std::endl << 
            __dump_trait_type(is_arithmetic,T) << std::endl << 
            __dump_trait_type(is_fundamental,T) << std::endl << 
            __dump_trait_type(is_object,T) << std::endl <<
            __dump_trait_type(is_scalar,T) << std::endl <<
            __dump_trait_type(is_compound,T) << std::endl <<
            __dump_trait_type(is_member_pointer,T) << std::endl <<
            __dump_trait_type(is_const,T) << std::endl <<
            __dump_trait_type(is_volatile,T) << std::endl <<
            __dump_trait_type(is_pod,T) << std::endl <<
            __dump_trait_type(is_empty,T) << std::endl <<
            __dump_trait_type(is_polymorphic,T) << std::endl <<
            __dump_trait_type(is_abstract,T) << std::endl <<
            __dump_trait_type(has_trivial_constructor,T) << std::endl <<
            __dump_trait_type(has_trivial_copy,T) << std::endl <<
            __dump_trait_type(has_trivial_assign,T) << std::endl <<
            __dump_trait_type(has_trivial_destructor,T) << std::endl <<
            __dump_trait_type(has_nothrow_constructor,T) << std::endl <<
            __dump_trait_type(has_nothrow_copy,T) << std::endl <<
            __dump_trait_type(has_nothrow_assign,T) << std::endl <<
            __dump_trait_type(has_virtual_destructor,T) << std::endl <<
            __dump_trait_type(is_signed,T) << std::endl <<
            __dump_trait_type(is_unsigned,T) << std::endl <<
            __dump_trait_value(rank,T); 

            return out;
        }
    };

#undef __dump_trait_type
#undef __dump_trait_value

} // namespace more

#endif /* _TRAITS_INFO_HH_ */

