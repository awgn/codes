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
#include <nullptr.hh>

int
main(int argc, char *argv[])
{
    char  * a = nullptr;
    if (a == nullptr)
    {
        std::cout << "ok" << std::endl;
    }
    return 0;
}

