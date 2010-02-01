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

struct uncopyable 
{
    uncopyable(const uncopyable &);
    uncopyable & operator=(const uncopyable &);

    uncopyable()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    uncopyable(const more::movable<uncopyable>)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~uncopyable()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

};


uncopyable
factory()
{
    uncopyable x;
    return more::move(x); 
}

void function(more::movable<uncopyable> a)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void function(uncopyable a)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int
main(int argc, char *argv[])
{
    uncopyable abc (factory());

    std::cout << "----------" << std::endl;
    
    function( more::movable<uncopyable>(abc) );

    std::cout << "----------" << std::endl;
    
    function( more::move(abc) );

    return 0;
}

