/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <lockless_queue.hpp>
#include <cassert>

// more::lockless_queue<int,128,3,1, std::mutex> queue;    
// more::lockless_queue<int,128,3,1, more::spinlock<more::policy::lock_smart<16> > > queue;    
more::lockless_queue<int,128,3,1, more::spinlock_backoff > queue;    

void thread_producer()
{
    for(int i=0; i<100000; ) 
    {
retry:
        if ( queue.push_back(0) ) {
            i++;
            continue;
        }
        goto retry;
    }
}

void thread_consumer()
{
    int r;
    for(;;) 
    {
        if (!queue.pop_front(r) )
        {
            sched_yield();
            continue;
        }
    }
}

int main()
{
    std::thread a(thread_producer);
    std::thread b(thread_producer);
    //std::thread c(thread_producer);
    //std::thread d(thread_consumer);

    a.join();
    b.join();
    //c.join();
    //d.join();
}
