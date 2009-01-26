/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <mmap.hh>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    more::mmap<PROT_WRITE|PROT_READ, MAP_ANONYMOUS| MAP_PRIVATE> Map("/dev/zero",1024);
    char * m = reinterpret_cast<char *>( Map() );
    strcpy(m, "Hello world!");
    std::cout << m << std::endl;
    return 0;
}
