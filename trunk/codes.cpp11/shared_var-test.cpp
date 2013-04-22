/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <shared_var.hpp>

#include <future>
#include <iostream>

int
main(int argc, char *argv[])
{
    more::shared_var<int> n(0);
    
    auto done = std::async(std::launch::async, [&]() 
    {
        for(int i = 0; i < 16; i++)
        {
            n.put(i);                       
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });

    while (done.wait_for(std::chrono::milliseconds(1)) != std::future_status::ready)
    {
        std::cout << n.get() << std::endl;
    }

    return 0;
}

