/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <typemap.hpp>

#include <string>
#include <typeinfo>
#include <cassert>
#include <iostream>

using namespace more::type;

    struct name     {};
    struct age      {};
    struct nickname {};
    struct null     {};
    struct address  {};

int
main(int argc, char *argv[])
{
    typedef typemap< std::pair<name, std::string>, std::pair<age, int>, std::pair<address, std::string> > map0; 

    get<map0, name>::type _me = "Nicola";
    get<map0, age>::type  _age = 38;

    assert( size<map0>::value == 3);

    typedef append<map0, nickname, std::string>::type map1;

    get<map1, nickname>::type _nick = "awgn";

    assert( size<map1>::value == 4);

    assert( (index_of<map0, name>::value     == 0) );
    assert( (index_of<map0, age>::value      == 1) );
    assert( (index_of<map0, address>::value  == 2) );
    assert( (index_of<map1, nickname>::value == 3) );

    return 0;
}
 
