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

class test_constness {

    int priv;

public:
    test_constness() 
    : priv(0) 
    {}

    // when returning a copy, only the const qualified
    // method is required...

    const int 
    get() const
    { return priv; }

    // when returning a reference to internal data,
    // both const and non-const qualified methods are required -- 
    // the const one makes use of the non-const implementation...
    //

    int &ref()
    { return priv; }

    const int &ref() const
    { return  const_cast<test_constness &>(*this).ref(); }

};


void foo(const test_constness &r)
{
    int a  = r.get();
    const int &b = r.ref();
}

int main(int argc, char *argv[])
{
    test_constness object;

    int a = object.get();
    const int b = object.get();
    int &c = object.ref();
    const int &d = object.ref();

    foo(object);

    return 0;
}
