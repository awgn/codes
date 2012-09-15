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

using namespace more;


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
    return closure(add,n);
}


void incr(int &n)
{
    n++;
}


struct moveable
{
    moveable()
    : count()
    {}
    
    moveable(const moveable &) = delete;
    moveable& operator=(const moveable &) = delete;

    moveable(moveable &&other)
    : count(other.count+1)
    {
        other.count = 0;
    }

    moveable& 
    operator=(moveable &&other)
    {
        count = other.count+1;
        other.count = 0;
        return *this;
    }

    int count;
};


int take(moveable &&m, int n)
{
    std::cout << "count: " << m.count << std::endl;
    return m.count;
}



int
main(int argc, char *argv[])
{
    auto f1 = curry(f0, 42, "hello");    
    auto f2 = curry(f1, 'x');
    f2(true); 

    auto add2 = factory_add(2);
    
    // test full curry 

    auto f3 = curry(add, 1, 2);

    assert(f3() == 3);

    assert(add2(40) == 42);

    int  n = 0;
    auto f = curry(incr, n);

    f();

    assert(n == 1);
    
    // err: closure with callable that takes
    //      arg by non-const l-value reference...
    // auto x = closure(incr, n);
    // x();
    
    moveable m;

    auto z = curry(&take, std::move(m), 0);

    z();

    return 0;
}
 
