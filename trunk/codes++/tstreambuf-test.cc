/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <tstreambuf.hh>
#include <pthread.h>

#include <atomicity-policy.hh>

void * thread(void *)
{
    for(int i = 0; i < 1000 ; i++) 
    {
        std::clog << "hello " << std::dec; 
        usleep(1000);
        std::clog << "world! (" << i << ")" <<  std::endl;
   
        /////////////////////////////////
        // which is almost equivalent to:
        
        // std::clog << more::lock_stream::lock() << "hello " << std::dec; 
        // usleep(1000);
        // std::clog << "world! (" << i << ")" <<  std::endl << more::lock_stream::unlock();
       
        // 
        // with the only difference that the spinlock is either stored in the tstreambuf<> 
        // or in the iword array of the ostream...

    }

    return NULL;
}


int
main(int argc, char *argv[])
{   
    // more::tstreambuf<atomicity::NONE::mutex> *b = new more::tstreambuf<atomicity::NONE::mutex>(std::cout.rdbuf());
    more::tstreambuf<more::tspinlock_half_recursive> *b = new more::tstreambuf<more::tspinlock_half_recursive>(std::cout.rdbuf());

    std::clog.rdbuf(b);

    pthread_t t1;
    pthread_t t2;
    pthread_t t3;

    pthread_create(&t1, NULL, thread, NULL);
    pthread_create(&t2, NULL, thread, NULL);
    pthread_create(&t3, NULL, thread, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    return 0;
}

