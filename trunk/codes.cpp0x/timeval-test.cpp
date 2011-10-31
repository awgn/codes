/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <timeval.hpp>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <chrono>
#include <thread>

#include <yats.hpp>

using namespace yats;
using namespace more;

Context(more_timeval_test)
{
    Test(timeval_now)
    {
        more::Timeval a = Timeval::now();
        Assert( static_cast<bool>(a), is_true() );
    }
    
    Test(timeval_null)
    {
        Timeval b;    
        Assert( static_cast<bool>(b), is_false() );
    }

    Test(timeval_streamable)
    {
        more::Timeval a = Timeval::now();
        std::cout <<  a << std::endl;    
    }

    more::Timeval a;
    more::Timeval b;
    more::Timeval c;

    Test(timeval_assignable)
    {
        a = Timeval::now();
        b = a;
    }

    Test(operations)
    {
        Timeval half(0,500000);
        b += half;

        Assert( (b - half) == a, is_true() );
    }

    Test(update)
    {
        b.update();
        std::this_thread::sleep_for(std::chrono::microseconds(500000));
        c = Timeval::now();
    
        Assert( c == c, is_true() );
        
        Assert( (c > b)  , is_true() );
        Assert( (c >= b) , is_true() );
        Assert( (c >= c) , is_true() );
        Assert( (c != b) , is_true() );   
        Assert( (b < c ) , is_true() );
        Assert( (b <= c) , is_true() );
        Assert( (b <= b) , is_true() );
    }

    Test(container)
    {
        std::vector<more::Timeval> vec;

        vec.push_back(c);
        vec.push_back(b);

        std::sort(vec.begin(), vec.end());
    }

    Test(swap)
    {
        more::Timeval b1 = b;
        more::Timeval c1 = c;
        std::swap(b,c);

        Assert( b1 == c, is_true() );
        Assert( c1 == b, is_true() );
    }
}
 
int
main(int argc, char *argv[])
{
    return yats::run();
}
