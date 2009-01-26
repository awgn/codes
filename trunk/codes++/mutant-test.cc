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
#include "mutant.hh"

struct base : public more::mutant
{
    int val;
    base(int n = 0)
    : val(n)
    {}

    virtual void hello() const = 0 ;
};

struct der1 : public base
{
    der1(int n = 0)
    : base(n) 
    {}

    virtual void hello() const
    {
        std::cout << "-> der1: value=" << val << std::endl;
    }
};

struct der2 : public base
{
    der2(int n = 0)
    : base(n)
    {}

    virtual void hello() const
    {
        std::cout << "-> der2: value=" << val << std::endl;
    }
};

struct xxx_1 : public der1
{};

struct xxx_2 : public der2
{};

int
main(int argc, char *argv[])
{
    base * b1 = new der1(1);
    base * b2 = new der2(2);

    std::cout << "runtime mutant object:\n";

    std::cout << "b1->hello(): " ; b1->hello();
    std::cout << "b2->hello(): " ; b2->hello();
    
    std::cout << "    b1->turn_into(der2());\n" ; b1->turn_into(der2());
    std::cout << "b1->hello(): " ; b1->hello();

    base *x = new xxx_1;

    std::cout << "runtime mutant polymorphism:\n";

    std::cout << "x->hello(): " ; x->hello();
    std::cout << "   x->turn_into(xxx_2());\n" ; x->turn_into(xxx_2());
    std::cout << "x->hello(): " ; x->hello();

    return 0;
}


