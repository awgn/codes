/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <lexical_cast.hpp>
#include <boost/lexical_cast.hpp>

#include <thread>
#include <iostream>

using namespace std;

const int max_iter = 10000000;

void
thread_routine_boost()
{
    auto b = chrono::system_clock::now();

    long long int sum = 0;
    for(int i=0; i< max_iter; ++i)
    {
        sum += boost::lexical_cast<int>("123456");
    }

    auto e = chrono::system_clock::now();
    auto diff = chrono::duration_cast<chrono::milliseconds>(e-b);

    std::cout << max_iter << " boost::lexical_cast<int>() " << diff.count() << " msec" << std::endl;
}

void
thread_routine_more()
{
    auto b = std::chrono::system_clock::now();

    long long int sum = 0;
    for(int i=0; i< max_iter; ++i)
    {
        sum += more::lexical_cast<int>("123456");
    }

    auto e = std::chrono::system_clock::now();
    auto diff = chrono::duration_cast<chrono::milliseconds>(e-b);

    std::cout << max_iter << " more::lexical_cast<int>() " << diff.count() << " msec" << std::endl;
}


int
main(int, char *[])
{
#ifndef _REENTRANT
    thread_routine_boost();
    thread_routine_more();
#else

    std::thread a(thread_routine_boost);
    std::thread b(thread_routine_boost);

    a.join();
    b.join();

    std::thread c(thread_routine_more);
    std::thread d(thread_routine_more);

    c.join();
    d.join();

#endif
    return 0;
}

