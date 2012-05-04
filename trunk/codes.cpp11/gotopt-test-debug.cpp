/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <vector>
#include <string>

#include <gotopt.hpp>

using namespace more::gotopt;

struct option opts[] = 
{
    option("Section:"),
    option('h', "hello", 1, "hello option"),
    option('w', "world", 0, "world option"),
    option('s', "abc"  , 0, nullptr)
};


int
main(int argc, char *argv[])
{
    std::vector<bool> context(128, false);
    
    context['a'] = true;
    // context['b'] = true;
    context['c'] = true;

    std::cout << _a <<  "," << _b << std::endl;

#ifndef __GXX_EXPERIMENTAL_CXX0X__ 
    typeof(_a|_b) r = _a | _b;    
    typeof(_a & _b) q = _a & _b;
    typeof(_a ^ _b) x = _a ^ _b;
#else
    auto r = _a | _b;
    auto q = _a & _b;
    auto x = _a ^ _b;
#endif

    std::cout << r << std::endl;
    std::cout << q << std::endl;
    std::cout << x << std::endl;

    std::cout << !( x | _b ) << std::endl;
    std::cout << eval ( !( _a & _b ) , context)  << std::endl;

    std::cout << "imply: " << ( _a >> _b ) << " -> " <<  eval( _a >> _b, context ) << std::endl;
    std::cout << "mutex: " << ( _a  % _b ) << " -> " <<  eval( _a  % _b, context ) << std::endl;

    more::gotopt::validate_expression( std::make_tuple(_true), context );  // default check...
    
    more::gotopt::validate_expression( std::make_tuple( 
                                       std::make_pair( _a >> (_b|_c), "-a -> -b or -c" ), 
                                       std::make_pair( _b % _a , "-b and -a are mutually exclusive")
                                    ), context );

    return 0;
}

