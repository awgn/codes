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

#include <yats.hpp>
using namespace yats;

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

Context(more_value_ptr_test)
{

    Test(transferable)
    {
        more::value_ptr<int, more::transfer_ptr> b (new int(2));
        more::value_ptr<int, more::transfer_ptr> c;
        
        Assert( static_cast<bool>(b.get()), is_true() );

        int * copy = b.get();
        c = b;

        Assert( static_cast<bool>(b.get()), is_false() );
        Assert( static_cast<bool>(c.get()), is_true() );
        Assert( c.get() == copy, is_true());
    }

    Test(value)
    {
        more::value_ptr<int,more::deepcopyable_ptr> a = new int(42);
        more::value_ptr<int,more::deepcopyable_ptr> x = a;

        Assert( *a, is_equal_to(42) );
        Assert( *x, is_equal_to(42) ); 
        Assert( a.get() != x.get(), is_true());
    }

    Test(cloneable)
    {
        // cloneable: object must provide
        // the virtual object * clone() const method

        more::value_ptr<base, more::cloneable_ptr> x =  new derived;
        more::value_ptr<base, more::cloneable_ptr> q = x;

        Assert( x.get() != q.get(), is_true());
    }
}
 
int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 
