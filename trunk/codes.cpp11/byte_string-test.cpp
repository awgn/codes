/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#include "byte_string.hpp"

int
main(int argc, char *argv[])
{
    more::byte_string<6> a("hello");
    more::byte_string<6> b("world");
    std::cout << (a < b) << std::endl;

    return 0;
}

