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
#include <tsafestream.hh>
#include <pthread.h>

void * thread(void *)
{
    for(int i = 0; i < 10 ; i++) 
    {
        std::cout << more::tsafestream::lock() << "[" << std::hex << pthread_self() << "] ";
        std::cout << "hello world! " << std::dec << i << std::endl;
        usleep(100000);
        std::cout << more::tsafestream::unlock(); 
    }

    return NULL;
}


int
main(int argc, char *argv[])
{
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

