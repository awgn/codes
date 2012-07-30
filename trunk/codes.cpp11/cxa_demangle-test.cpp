/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <cxa_demangle.hpp>
#include <iostream>

#include <yats.hpp>

using namespace yats;

Context(demangle_test)
{
    Test(int)
    {
        Assert(more::cxa_demangle(typeid(int).name()), is_equal_to(std::string("int")));
    }

    struct class_in_namespace {};

    Test(class_in_namespace)
    {
        Assert(more::cxa_demangle(typeid(class_in_namespace).name()), 
               is_equal_to(std::string("demangle_test::class_in_namespace")));
    }
}

int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 
