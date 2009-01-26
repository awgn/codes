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
#include "functor.hh"

int freefunction(int p) 
{
    std::cout << __PRETTY_FUNCTION__ << ": " << p << std::endl;
    return 0;
}

struct aclass {

    int operator()(int p) const
    {
        std::cout << __PRETTY_FUNCTION__ << ": " << p << std::endl;
        return 0;
    }        

};


void foo(const generic::functor<int,int> &ref)
{
    ref(3);
}

int main()
{
    aclass test;

    generic::functor<int,int> hello1(&freefunction);    
    generic::functor<int,int> hello2(test);    
    
    hello1(1);
    hello2(2);

    foo(hello2);
}

