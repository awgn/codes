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
#include <spinlock.hh>  // more!

#include <iostream>
#include <cassert>

// typedef more::spinlock< more::lock_smart<64> > spinlock_type;
// typedef more::spinlock< more::lock_relaxed   > spinlock_type;
// typedef more::spinlock< more::lock_aggressive > spinlock_type;
typedef more::spinlock< more::lock_backoff<32> > spinlock_type;

spinlock_type ticket_lock;

volatile int g = 0;

void *thread_producer(void *)
{
    for(int i=0; i < 500000; i++) 
    {
        more::scoped_lock< spinlock_type > _s_(ticket_lock);
        g++;
    }    
    return 0;
}

void *thread_consumer(void *)
{
    for(int i=0; i < 500000; i++) 
    {
        more::scoped_lock< spinlock_type > _s_(ticket_lock);
        g--; 
    }    
    return 0;
}

int
main(int argc, char *argv[])
{
    pthread_t a,b,c,d;

    pthread_create(&a, NULL, thread_producer, NULL);
    pthread_create(&b, NULL, thread_consumer, NULL);
    
    pthread_create(&c, NULL, thread_producer, NULL);
    pthread_create(&d, NULL, thread_consumer, NULL);

    pthread_join(a,NULL);
    pthread_join(b,NULL);
    pthread_join(c,NULL);
    pthread_join(d,NULL);

    assert( g == 0 );
    return 0;
}

