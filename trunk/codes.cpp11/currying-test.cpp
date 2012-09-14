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

void fun(int a, std::string b, char c, bool q)
{
    std::cout << std::boolalpha << a << ' ' << b << ' ' << c << ' ' << q << std::endl;
}


int add(int a, int  b)
{
    return a+b;
}


std::function<int(int)> factory_add(int n)
{
    return more::closure(add,n);
}


int
main(int argc, char *argv[])
{
    auto f = more::curry(fun, 42, "hello");    
    auto q = more::curry(f, 'x');
    
    q(true); 

    auto add2 = factory_add(2);

    std::cout << add2(40) << std::endl;

    return 0;
}
 
