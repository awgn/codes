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
    {
        value = rhs.value;
        rhs.value = 0;
    }

    test& operator=(test &&rhs)
    {
        super::operator=(std::move(rhs));
        
        value = rhs.value;
        rhs.value = 0;
        return *this;
    }
};


  int
main(int argc, char *argv[])
{
    test x;
    x.value = 42;

    more::entangled_ptr<test> p = x.entangled_from_this();

    assert( p.get() == &x );
    assert( p->value == x.value);

    std::cout << "p: " << (void *)p.get() << " " << p->value << std::endl; 

    test y = std::move(x);

    std::cout <<  "p: " << (void *)p.get() << " " << p->value << std::endl; 

    assert( p.get() == &y);
    assert( p->value == y.value);

    more::entangled_ptr<test> q = y.entangled_from_this();
    assert( q.get() == &y);
    assert( q->value == y.value);
    
    std::cout <<  "p: " << (void *)p.get() << " " << p->value << std::endl; 
    std::cout <<  "q: " << (void *)q.get() << " " << q->value << std::endl; 
    
    test z = std::move(y);

    assert( p->value == z.value);
    assert( q->value == z.value);

    more::entangled_ptr<test> w(p);

    assert( w.get() == &z);
    assert( w->value == z.value);

    std::cout <<  "p: " << (void *)p.get() << " " << p->value << std::endl; 
    std::cout <<  "q: " << (void *)q.get() << " " << q->value << std::endl; 
    std::cout <<  "w: " << (void *)w.get() << " " << w->value << std::endl; 

    assert( p.use_count() == 3);
    assert( q.use_count() == 3);
    assert( w.use_count() == 3);

    {
        test error;
        error.value = 11;
        w = error.entangled_from_this();
    }

    std::cout << "p: " << (void *)p.get() << " " << p->value << std::endl; 
    std::cout << "q: " << (void *)q.get() << " " << q->value << std::endl; 
    try 
    {
        std::cout << "w: " << (void *)q.get() << " " << w->value << std::endl; 
    }
    catch(...)
    {
        std::cout << "done." << std::endl;
    }

    return 0;
}
 
