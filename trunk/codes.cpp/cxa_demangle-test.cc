/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <cxa_demangle.hh>
#include <iostream>

int
main(int, char *[])
{
    struct cpp_demangled_struct_name {} a;
    std::cout << more::cxa_demangle(typeid(a).name()) << std::endl; 
    return 0;
}
 
