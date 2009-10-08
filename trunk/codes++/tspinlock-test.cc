/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <tspinlock.hh>
#include <iostream>
#include <pthread.h>

more::tspinlock ticket_lock;

// pthread_spinlock_t spin_lock;
// pthread_mutex_t    mutex_lock;

int g = 0;

void *thread_1(void *)
{
    for(int i=0; i < 1000000; i++) 
    {
        more::scoped_tspinlock _s_(ticket_lock);
        // pthread_spin_lock(&spin_lock);
        // pthread_mutex_lock(&mutex_lock);
        g++;
        // pthread_mutex_unlock(&mutex_lock);
        // pthread_spin_unlock(&spin_lock);
    }    
}

void *thread_2(void *)
{
    for(int i=0; i < 1000000; i++) 
    {
        more::scoped_tspinlock _s_(ticket_lock);
        //pthread_spin_lock(&spin_lock);
        // pthread_mutex_lock(&mutex_lock);
        g--; 
        // pthread_mutex_unlock(&mutex_lock);
        //pthread_spin_unlock(&spin_lock);
    }    
}

int
main(int argc, char *argv[])
{
    pthread_t a,b;

    pthread_create(&a, NULL, thread_1, NULL);
    pthread_create(&b, NULL, thread_2, NULL);
    
    pthread_join(a,NULL);
    pthread_join(b,NULL);

    std::cout << "g = "  << g << " (zero is ok)"<< std::endl;
    return 0;
}
 
