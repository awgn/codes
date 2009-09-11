/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <errcode.h>

#include <iostream>
#include <string>
#include <cassert>

int
main(int argc, char *argv[])
{
    assert( std::string("ENOTRECOVERABLE") == more::errcode_str[ENOTRECOVERABLE] );
    assert( std::string("ENOTRECOVERABLE") == more::strerrcode(ENOTRECOVERABLE) );

    try {
        more::strerrcode(180);
    }
    catch(...) {
        std::cout << "ok." << std::endl;
    }

    return 0;
}
 
