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
#include <fstream>
#include <pthread.h>

#include <tstreambuf.hh>
#include <atomicity-policy.hh>

void * thread(void *)
{
    for(int i = 0; i < 10000 ; i++) 
    {
        std::clog << "hello " << std::dec; 
        std::clog << 0 << ' ';
        std::clog << 1 << ' ';
        std::clog << 2 << ' ';
        std::clog << 3 << ' ';
        std::clog << 4 << ' ';
        std::clog << 5 << ' ';
        std::clog << 6 << ' ';
        std::clog << 7 << ' ';
        std::clog << 8 << ' ';
        std::clog << 9 << ' ';
        std::clog << "world! (" << i << ")" <<  std::endl;
   
        /////////////////////////////////
        // which is almost equivalent to:
        
        // std::clog << more::lock_stream::lock() << "hello " << std::dec; 
        // ...
        // std::clog << "world! (" << i << ")" <<  std::endl << more::lock_stream::unlock();
       
        // with the only difference that the spinlock is stored in the tstreambuf<> 
        // instead of in the iword array of the ostream...

    }

    return NULL;
}



int
main(int argc, char *argv[])
{   
    std::ofstream test_txt("test.txt");

    more::tstreambuf<more::spinlock_open_recursive> *b = new more::tstreambuf<more::spinlock_open_recursive>(test_txt.rdbuf());
    // more::tstreambuf<more::spinlock_open_recursive> *b = new more::tstreambuf<more::spinlock_open_recursive>(std::cout.rdbuf());
    // std::clog.rdbuf(b);

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

    test_txt.close();
    std::clog.rdbuf(NULL);

    return 0;
}

