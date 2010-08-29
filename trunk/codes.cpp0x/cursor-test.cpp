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
#include <cursor.hpp>

int
main(int argc, char *argv[])
{
    char buffer[4];

    more::cursor<short> cur(buffer, buffer+4);
    more::cursor<const short> cur2(cur);    // ok, cctor of CV-qualified type
        
    * cur++ = 0;
    * cur++ = 1;
    // * cur++ = 2;

    std::cout << "size: " << cur.size() << " bytes left." << std::endl;

    const char c_buffer[4] = { '\0' };

    // more::cursor<short> c_err(c_buffer, c_buffer+4);  // ok, error detected.
    more::cursor<const short> c_cur(c_buffer, c_buffer+4);

    // * c_cur = 10; ok, error detected.

    std::cout << "*c_cur++ = " << * c_cur++ << std::endl;
    std::cout << "*c_cur++ = " << * c_cur++ << std::endl;

    // std::cout << "*c_cur++ = " << * c_cur++ << std::endl; // range error, detected.

    std::cout << "done." << std::endl;
    return 0;
}
 
