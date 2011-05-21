/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <double_checked_sinlegon.hpp>

#include <iostream>
#include <cassert>

typedef more::double_checked::singleton<int> single_int;

int
main(int argc, char *argv[])
{
    int & n = single_int::instance(42);

    assert(n == 42);
    assert(single_int::instance() == 42);

    return 0;
}
 
