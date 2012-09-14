/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <currying.hpp>

#include <string>
#include <iostream>
#include <functional>

#include <cassert>

void f0(int a, std::string b, char c, bool q)
{
    std::cout << std::boolalpha << a << ' ' << b << ' ' << c << ' ' << q << '!' << std::endl;
}


int add(int a, int  b)
{
    return a+b;
}


std::function<int(int)> factory_add(int n)
{
    return more::closure(add,n);
}


void incr(int &n)
{
    n++;
}


int
main(int argc, char *argv[])
{
    auto f1 = more::curry(f0, 42, "hello");    
    auto f2 = more::curry(f1, 'x');
    f2(true); 

    auto add2 = factory_add(2);

    assert(add2(40) == 42);

    int  n = 0;
    auto f = more::curry(incr, n);

    f();

    assert(n == 1);
    
    // err: closure with callable that takes
    //      arg by non-const l-value reference...
    // auto x = more::closure(incr, n);
    // x();

    return 0;
}
 
