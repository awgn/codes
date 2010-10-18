/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <entangled_ptr.hpp>

#include <iostream>
#include <cassert>

struct test : public more::enable_entangled_from_this<test>
{
    int value;

    test() = default;

    test(test &&rhs)
    : super(std::move(rhs))
    {}
};


  int
main(int argc, char *argv[])
{
    test x;
    x.value = 42;

    more::entangled_ptr<test> p = x.entangled_from_this();

    assert( p.get() == &x );
    assert( p->value == x.value);

    std::cout << (void *)p.get() << std::endl;

    test y = std::move(x);

    std::cout << (void *)p.get() << std::endl;

    assert( p.get() == &y);
    assert( p->value == y.value);

    more::entangled_ptr<test> q = y.entangled_from_this();
    assert( q.get() == &y);
    assert( q->value == y.value);

    test z = std::move(y);

    assert( p->value == z.value);
    assert( q->value == z.value);

    more::entangled_ptr<test> w(p);

    assert( w.get() == &z);
    assert( w->value == z.value);

    assert( w.use_count() == 3);

    return 0;
}
 
