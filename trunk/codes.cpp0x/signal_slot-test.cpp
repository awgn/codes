#include <signal_slot.hpp>

#include <iostream>
#include <cassert>

struct hello
{
    hello(int x)
    : value(x), method(this, &hello::__method)
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
main(int argc, char *argv[])
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
 
