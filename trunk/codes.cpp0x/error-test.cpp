/* $Id: error-test.cc 466 2010-03-13 12:31:43Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <error.hpp>

#include <iostream>
#include <cassert>

int
main(int argc, char *argv[])
{
    for(int i=0; i < 132; i++) 
    {
        std::cout << more::pretty_strerror(i) << std::endl; 
    }

    for(int i=0; i > -15; i--) 
    {
        std::cout << more::pretty_gai_strerror(i) << std::endl; 
    }

    try 
    {
        throw more::syscall_error("test",EINVAL);
    }
    catch(more::syscall_error &e)
    {
        assert(e.code() == EINVAL);
    }

    return 0;
}
 
