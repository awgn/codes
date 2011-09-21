 /* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <maybe.hpp>

#include <cassert>
#include <iostream>

#include <yats.hpp>
using namespace yats;

Context(maybe_test)
{
    Maybe<int> x;

    Test(nothing)
    {
        Assert(Nothing == Nothing);
        Assert(Nothing == x);
        Assert(x == Nothing);
        Assert(x == x);
    }

    Maybe<int> y(1);
    
    Test(just)
    {
        Assert(y != Nothing);
        Assert(Nothing != y);
        
        Assert(y != x);
        Assert(x != y);

        Assert(y == y);
        Assert(y == Just(1));
        Assert(Just(1) == y);
    }

    // Assert(y == 1);  only explicit conversion..
    // Assert(1 == y);

    Test(explicit_conversion)
    {
        Assert(static_cast<int>(y) == 1);  // only explicit conversion..
    }    

    Test(maybe_assign)
    {
        x = Just(1);

        Assert(y == x);
        Assert(x == y);
    }

    Maybe<int> w(Nothing);
    
    Test(copy_constructor)
    {
        Maybe<int> z(x);
        
        Assert( x == z );
        Assert( z == x );

        Assert(w == Nothing);
        Assert(Nothing == w);
    }

    // Assert(!q);  deleted
    // Assert(q);   deleted
    
    Maybe<int> q(Just(0));

    Test(copy_constructor_extra)
    {
        Assert(q.value() == 0);
        Assert(q == Just(0));
        Assert(q != Just(1));
        Assert(q != Nothing);

        Assert(Nothing != Just(1));
    }
}

int
main(int argc, char *argv[])
{
    return yats::run();
}

