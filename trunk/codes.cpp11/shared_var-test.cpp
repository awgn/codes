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
main(int, char *[])
{
    more::shared_var<int> n(0);

    auto done = std::async(std::launch::async, [&]()
    {
        auto ptr = n.get();

        for(int i = 1; i <= 1000; i++)
        {
            auto np = n.safe_put(ptr, i);
            if (np.first)
            {
                ptr = np.second;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            else
            {
                throw std::runtime_error("internal error");
            }
        }
    });

    auto p = n.get();
    while (done.wait_for(std::chrono::microseconds(500)) != std::future_status::ready)
    {
        auto q = n.get();
        if (q != p)
        {
            p = q;
            std::cout << *q << std::endl;
        }
    }

    return 0;
}

