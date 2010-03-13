/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <scoped_raii.hh>
#include <iostream>
#include <string>

struct fake_mutex
{
    void lock()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    void unlock()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    bool try_lock()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
        return true;
    }
};


int
main(int argc, char *argv[])
{
    fake_mutex m;

    more::raii::scoped_lock<fake_mutex> lock(m);
    more::raii::scoped_try_lock<fake_mutex> trylock(m);

    return 0;
}
 
