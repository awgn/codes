/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <shared_queue.hpp>
#include <iostream>
#include <cassert>
#include <chrono>
#include <atomic>

using namespace more;

more::shared_queue<int, 128, multiple_producer, single_consumer> queue;    


std::atomic_bool stop(false);


void thread_producer()
{
    for(int i=0; i<1000000; ) {
retry:
        if ( queue.push_back(0) ) {
            i++;
            std::this_thread::yield();
            continue;
        }
        goto retry;
    }

    std::cout << "push done!" << std::endl;
}

void thread_consumer()
{
    int n, r = 0;
    for(;;) {

        if (!queue.pop_front(n) )
        {
            if (stop)
                break;   

            std::this_thread::yield();
            continue;
        }
        r++;
    }

    std::cout << "-> consumed " << r << " elements" << std::endl;
}

int main()
{
    std::thread a(thread_producer);
    std::thread b(thread_producer);
    std::thread c(thread_producer);
    std::thread d(thread_consumer);

    a.join();
    b.join();
    c.join();

    std::cout << "waiting for consumer to terminate..." << std::endl;

    stop.store(true);
    d.join();
}
