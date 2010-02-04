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
#include <exprtempl.hh>

using namespace more::expr;

int
main(int argc, char *argv[])
{
    std::cout << std::boolalpha;
    std::cout << (( !_false ^ _true ) == _false) << " -> " << eval( ( !_false ^ _true ) == _false ) << std::endl;
    return 0;
}

