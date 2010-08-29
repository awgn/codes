/* $Id: colorful-test.cc 466 2010-03-13 12:31:43Z nicola.bonelli $ */
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
    std::cout << colorful< TYPELIST(ecma::bold, ecma::fg::red) >()  << "hello"  << 
                 colorful< TYPELIST(ecma::bold, ecma::fg::blue) >() << " world" << colorful< TYPELIST(ecma::reset) >() << "!\n";
    return 0;
}
 
