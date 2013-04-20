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

struct noncopyable 
{
    noncopyable(const noncopyable &);
    noncopyable & operator=(const noncopyable &);

    noncopyable()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    noncopyable(const more::movable<noncopyable>)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~noncopyable()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

};


noncopyable
factory()
{
    noncopyable x;
    return more::move(x); 
}

void function(more::movable<noncopyable>)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

void function(noncopyable)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

int
main(int, char *[])
{
    noncopyable abc (factory());

    std::cout << "----------" << std::endl;
    
    function( more::movable<noncopyable>(abc) );

    std::cout << "----------" << std::endl;
    
    function( more::move(abc) );

    return 0;
}

