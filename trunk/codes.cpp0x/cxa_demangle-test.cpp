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
#include <cassert>

int
main(int argc, char *argv[])
{
    struct cpp_demangled_struct_name {} a;
    std::cout << more::cxa_demangle(typeid(a).name()) << std::endl;
    assert(more::cxa_demangle(typeid(a).name()) == "main::cpp_demangled_struct_name");
    return 0;
}
 
