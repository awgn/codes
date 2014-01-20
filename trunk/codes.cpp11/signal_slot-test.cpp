/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <signal_slot.hpp>

#include <iostream>
#include <cassert>

struct hello
{
    hello(int x)
    : method(this, &hello::__method), value(x)
    {}

    more::slot<int(int)> method;
    int __method(int n)
    {
        std::cout << "object:" << value << " arg:" << n << std::endl;
        return n+1;
    }

    int value;
};

int
main(int, char *[])
{
    hello x(0);
    hello y(1);

    more::signal<int(int)> sig;

    sig.connect(x.method);
    sig.connect(y.method);

    // assert(x.method(0) == 1);

    sig(0);

    sig.disconnect(y.method);

    sig(1);

    return 0;
}

