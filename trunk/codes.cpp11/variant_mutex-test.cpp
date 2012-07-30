/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <variant_mutex.hpp>

#include <iostream>
#include <mutex>

struct null_mutex  
{
    null_mutex()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
    
    ~null_mutex()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    void lock()
    {
    }

    void unlock()
    {
    }

    bool try_lock()
    {
        return true;
    }
};

typedef more::variant_mutex<null_mutex, std::mutex> mutex;

int
main(int, char *[])
{
    mutex x(0);

    for(int i=0; i < 1000000000; i++)
    {
        std::lock_guard<mutex> lock(x); 
    }

    return 0;
}
 
