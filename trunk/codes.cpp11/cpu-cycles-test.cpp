/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <cpu-cycles.hpp>
#include <iostream>

int
main(int, char *[])
{
    std::cout << "tsc: " << more::this_cpu::get_cycles() << std::endl;
    std::cout << "tsc: " << more::this_cpu::get_cycles() << std::endl;
    return 0;
}
 
