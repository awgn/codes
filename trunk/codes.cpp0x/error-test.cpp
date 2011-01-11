/* $Id$ */
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
#include <yats.hpp>
using namespace yats;

Context(syscall_error_class)
{
    Setup(init)
    {    
        for(int i=0; i < 132; i++) 
        {
            std::cout << more::pretty_strerror(i) << std::endl; 
        }

        for(int i=0; i > -15; i--) 
        {
            std::cout << more::pretty_gai_strerror(i) << std::endl; 
        }
    }

    Test(exception)
    {
        auto e = more::syscall_error("test", EINVAL);

        Assert(e.what(), is_equal_to(std::string("test: Invalid argument [EINVAL]")));
        Assert(e.code(), is_equal_to(EINVAL));
    }
}
 
int
main(int argc, char *argv[])
{
    return yats::run();
}
