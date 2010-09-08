/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
   
#include <print.hpp>

#include <thread>
#include <cstdio>

struct tp
{
    void operator()() const
    {
        more::print(std::cout, "%1\n", std::this_thread::get_id());
    }
};


int
main(int argc, char *argv[])
{
    more::print(std::cout, "%1 %2\n", "hello", std::string("world"));
    more::print(std::cout, "0x%1\n", more::flags<std::ios::hex>(3735928559));
    more::print(std::cout, "%1\n", std::thread::id());

    std::thread t((tp()));
    t.join();

    return 0;
}
 
