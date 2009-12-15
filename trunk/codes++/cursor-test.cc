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
#include <cursor.hh>

int
main(int argc, char *argv[])
{
    char buffer[4];

    more::cursor<short> cur(buffer, buffer+4);

    * cur++ = 0;
    * cur++ = 1;
    // * cur++ = 2;

    std::cout << "size: " << cur.size() << " bytes left." << std::endl;

    return 0;
}
 
