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
#include <format.hpp>

int
main(int argc, char *argv[])
{
    std::string hello("hello");

    std::cout << more::format("%1 %2 %3... %%%4 %5%%!") % 1 % 2 % 3 % hello % "world" << std::endl;

    std::string h = more::format("%1 %2 %3... %%%4 %5%%!") % 1 % 2 % 3 % hello % "world";
    std::cout << h << std::endl;

    return 0;
}
    
