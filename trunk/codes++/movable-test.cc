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
#include <movable.hh>

void function(int a)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::cout << "value: " << a << std::endl;
}

void function(more::movable<int> a)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    std::cout << "value: " << a << std::endl;
}


struct simple
{
    simple(int n)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};


struct simple_movable 
{
    simple_movable(int n)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    simple_movable(more::movable<int> k)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};


int factory()
{
    int n = 0;
    return more::move(n);
}


struct uncopyable 
{
    uncopyable(const uncopyable &);
    uncopyable & operator=(const uncopyable &);

    uncopyable()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    uncopyable(more::movable<uncopyable>)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~uncopyable()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

};


uncopyable
factory2()
{
    uncopyable x;
    return more::move(x); 
}


int
main(int argc, char *argv[])
{
    int n = 10;

    more::movable<int> m(n);   

    function(m);

    simple x(m);

    simple_movable y(m);
    simple_movable z(m);
    simple_movable(factory());

    std::cout << "----------" << std::endl;

    uncopyable abc (factory2());

    return 0;
}

