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
#include <colorful.hpp>

using namespace more;

int
main(int argc, char *argv[])
{
    std::cout << colorful<ecma::bold, ecma::fg::red>()  << "hello"  << 
                 colorful<ecma::bold, ecma::fg::blue>() << " world" << 
                 colorful<ecma::reset>() << "!\n";
    return 0;
}
 
