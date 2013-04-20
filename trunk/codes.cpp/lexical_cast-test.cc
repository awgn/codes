/* $Id: lexical_cast.hh 445 2010-02-10 21:15:08Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <lexical_cast.hh>
#include <boost/lexical_cast.hpp>

#include <timeval.hh>

#ifdef _REENTRANT
#include <pthread.h>
#endif

const int max_iter = 10000000;

void *
thread_routine_boost(void *)
{
    more::Timeval b = more::Timeval::now();

    for(int i=0; i< max_iter; ++i) 
    {
        boost::lexical_cast<int>("123456");
    }

    more::Timeval e = more::Timeval::now();
    std::cout << max_iter << " boost::lexical_cast<int>() " << (e-b).to_msec() << " msec" << std::endl;
    
    return 0;
}

void *
thread_routine_more(void *)
{    
    more::Timeval b = more::Timeval::now();

    long long int a = 0;
    for(int i=0; i< max_iter; ++i) 
    {
        a += more::lexical_cast<long long int>("123456");
    }

    more::Timeval e = more::Timeval::now();
    std::cout << max_iter << " more::lexical_cast<int>() " << (e-b).to_msec() << " msec" << std::endl;
    std::cout << a << std::endl;
    return 0;
}


int
main(int, char *[])
{
#ifndef _REENTRANT
    thread_routine_boost(0);
    thread_routine_more(0);
#else

    pthread_t a,b,c;
    pthread_create(&a, NULL, thread_routine_boost, NULL);
    pthread_create(&b, NULL, thread_routine_boost, NULL);
    // pthread_create(&c, NULL, thread_routine_boost, NULL);

    pthread_join(a,NULL);
    pthread_join(b,NULL);
    // pthread_join(c,NULL);

    pthread_create(&a, NULL, thread_routine_more, NULL);
    pthread_create(&b, NULL, thread_routine_more, NULL);
    // pthread_create(&c, NULL, thread_routine_more, NULL);

    pthread_join(a,NULL);
    pthread_join(b,NULL);
    // pthread_join(c,NULL);

#endif
    return 0;
}
 
