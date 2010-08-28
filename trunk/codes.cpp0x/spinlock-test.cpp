/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <spinlock.hpp>  // more!

#include <iostream>
#include <thread>
#include <cassert>

// typedef more::spinlock< more::policy::lock_usleep<100> > spinlock_type;
// typedef more::spinlock< more::policy::lock_aggressive >  spinlock_type;
// typedef more::spinlock< more::policy::lock_yield >       spinlock_type;
typedef more::ticket_spinlock< more::policy::lock_smart<1024> > spinlock_type;

spinlock_type lock;

volatile int g = 0;

void thread_producer()
{
    for(int i=0; i < 500000; i++) 
    {
        std::lock_guard<spinlock_type> _s_(lock);
        g++;
    }    
}

void thread_consumer()
{
    for(int i=0; i < 500000; i++) 
    {
        std::lock_guard<spinlock_type> _s_(lock);
        g--; 
    }    
}

int
main(int argc, char *argv[])
{
    std::thread a(thread_producer);
    std::thread b(thread_consumer);
    std::thread c(thread_producer);
    std::thread d(thread_consumer);

    a.join();
    b.join();
    c.join();
    d.join();

    assert( g == 0 );
    return 0;
}

