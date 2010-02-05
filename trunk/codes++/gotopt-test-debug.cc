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

#include <gotopt.hh>

using namespace more::gotopt;

struct option opts[] = 
{
    option("Section:"),
    option('h', "hello", 1, "hello option"),
    option('w', "world", 0, "world option"),
    option('s', "abc"  , 0, NULL)
};


int
main(int argc, char *argv[])
{
    std::vector<bool> context(128, false);
    
    context['a'] = true;
    // context['b'] = true;
    context['c'] = true;

    std::cout << _a <<  "," << _b << std::endl;

    // binary_expr<opt, opt, op_or> r(a,b);

    typeof(_a|_b) r = _a | _b;

    // binary_expr<opt, opt, op_and> q(a,b);

    typeof(_a & _b) q = _a & _b;
    typeof(_a ^ _b) x = _a ^ _b;

    std::cout << r << std::endl;
    std::cout << q << std::endl;
    std::cout << x << std::endl;

    std::cout << !( x | _b ) << std::endl;
    std::cout << eval ( !( _a & _b ) , context)  << std::endl;

    std::cout << "imply: " << ( _a >> _b ) << " -> " <<  eval( _a >> _b, context ) << std::endl;
    std::cout << "mutex: " << ( _a  % _b ) << " -> " <<  eval( _a  % _b, context ) << std::endl;

    more::gotopt::validate_expression( std::tr1::make_tuple(_true), context );  // default check...
    
    more::gotopt::validate_expression( std::tr1::make_tuple( 
                                       std::make_pair( _a >> (_b|_c), "-a -> -b or -c" ), 
                                       std::make_pair( _b % _a , "-b and -a are mutually exclusive")
                                    ), context );

    return 0;
}

