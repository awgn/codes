/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <pthread.h>

#include <shared_queue.hh>
#include <cassert>

// more::shared_queue<int,128,3,1, more::atomicity::DEFAULT::mutex> queue;    
more::shared_queue<int,128,3,1, more::spinlock<more::lock_smart<16> > > queue;    
// more::shared_queue<int,128,3,1, more::spinlock<more::lock_relaxed > > queue;    

void *thread_producer(void *)
{
    for(int i=0; i<10000000; ) {

retry:
        if ( queue.push_back(0) ) {
            i++;
            continue;
        }
        goto retry;
    }

    return NULL;
}

void *thread_consumer(void *)
{
    int r;
    for(;;) {
        if (!queue.pop_front(r) )
        {
            sched_yield();
            continue;
        }
    }

    return NULL;
}

int main(int, char *[])
{
    pthread_t a;
    pthread_t b;
    pthread_t c;
    pthread_t d;

    pthread_create(&a, NULL, thread_producer, NULL);
    pthread_create(&b, NULL, thread_producer, NULL);
    pthread_create(&c, NULL, thread_producer, NULL);
    pthread_create(&d, NULL, thread_consumer, NULL);

    pthread_join(a,NULL);
    pthread_join(b,NULL);
    pthread_join(c,NULL);
    pthread_join(d,NULL);
}
