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
#include <value_ptr.hpp>

struct base 
{
    virtual 
    base * clone() const = 0; 
};

struct derived : public base
{
    derived *
    clone() const
    {
        return new derived(*this);
    }
};

int
main(int argc, char *argv[])
{
    more::value_ptr<int> a(new int(1));

    more::value_ptr<int, more::transfer_ptr> b(new int(2));
    std::cout << "b:" << static_cast<void *>(b.get()) << std::endl;

    more::value_ptr<int, more::transfer_ptr> c(b);
    
    std::cout << "b:" << static_cast<void *>(b.get()) << std::endl;
    std::cout << "c:" << static_cast<void *>(c.get()) << std::endl;

    b = c;    

    std::cout << "b:" << static_cast<void *>(b.get()) << std::endl;
    std::cout << "c:" << static_cast<void *>(c.get()) << std::endl;

    more::value_ptr<int,more::deepcopyable_ptr> abc ( new int(10) );
    more::value_ptr<int,more::deepcopyable_ptr> xxx = abc;

    std::cout << *xxx << std::endl;
    std::cout << *abc << std::endl;

    * xxx = 20;
    * abc = 30;

    std::cout << *xxx << std::endl;
    std::cout << *abc << std::endl;

    // cloneable: object must provide
    // the virtual object * clone() const method

    more::value_ptr<base, more::cloneable_ptr> x( new derived );
    more::value_ptr<base, more::cloneable_ptr> q = x;

    return 0;
}
 
