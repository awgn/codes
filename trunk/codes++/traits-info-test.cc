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
#include <traits-info.hh>

struct null 
{
    null(const null &) throw()
    {}
};

int
main(int argc, char *argv[])
{
    std::cout << more::traits_info<null>() << std::endl;
    return 0;
}
 
