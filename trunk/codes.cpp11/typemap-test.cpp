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

#include <yats.hpp>

using namespace yats;
using namespace more::type;

    struct name     {};
    struct age      {};
    struct nickname {};
    struct null     {};
    struct address  {};

Context(more_typemap_test)
{
    typedef typemap< std::pair<name, std::string>, std::pair<age, int>, std::pair<address, std::string> > map0; 

    Test(size)
    {
        get<map0, name>::type _me = "Nicola";
        get<map0, age>::type  _age __attribute__((unused)) = 38;

        Assert( static_cast<int>(size<map0>::value), is_equal_to(3));
    }

    typedef append<map0, nickname, std::string>::type map1;

    Test(append)
    {
        get<map1, nickname>::type _nick = "awgn";
        Assert( static_cast<int>(size<map1>::value), is_equal_to(4));
    }
     

    Test(index_of)
    {
        Assert( static_cast<int>(index_of<map0, name>::value    ) , is_equal_to(0) );
        Assert( static_cast<int>(index_of<map0, age>::value     ) , is_equal_to(1) );
        Assert( static_cast<int>(index_of<map0, address>::value ) , is_equal_to(2) );
        Assert( static_cast<int>(index_of<map1, nickname>::value) , is_equal_to(3) );
    }
 
}
int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
