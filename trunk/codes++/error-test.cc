/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <error.hh>

#include <iostream>

int
main(int argc, char *argv[])
{
    for(int i=0; i < 132; i++) {
        std::cout << more::pretty_strerror(i) << std::endl; 
    }

    throw more::syscall_error("prova",EINVAL);

    return 0;
}
 
