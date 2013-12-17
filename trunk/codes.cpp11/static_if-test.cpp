/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <type_traits>
#include <iostream>

#include <static_if.hpp>

struct function
{
    template <typename T>
    void operator()(T n)
    {
        std::cout << n << std::endl;
    }
};


int
main(int argc, char *argv[])
{
    static_if<true>().then_<function>(42)
                     .else_<function>("hello world");
    
    static_if<false>().then_<function>(42)
                      .else_<function>("hello world");

     return 0;
}

