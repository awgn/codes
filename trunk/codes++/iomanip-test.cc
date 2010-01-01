 /* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <iomanip.hh>
#include <iostream>

int
main(int argc, char *argv[])
{
    int a;
    std::cin >> more::ignore_line >> a;
    std::cout << more::spaces(42) << a;

    return 0;
}
 
