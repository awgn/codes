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

int
main(int argc, char *argv[])
{
    Maybe<int> x;

    assert(Nothing == Nothing);
    assert(Nothing == x);
    assert(x == Nothing);
    assert(x == x);

    Maybe<int> y(1);

    assert(y != Nothing);
    assert(Nothing != y);
    
    assert(y != x);
    assert(x != y);

    assert(y == y);
    assert(y == Just(1));
    assert(Just(1) == y);

    assert(y == 1);
    assert(1 == y);

    x = Just(1);

    assert(y == x);
    assert(x == y);

    Maybe<int> z(x);
    Maybe<int> w(Nothing);
    Maybe<int> q(Just(0));

    assert( x == z );
    assert( z == x );

    assert(w == Nothing);
    assert(Nothing == w);

    // assert(!q);  deleted
    // assert(q);   deleted

    assert(q == 0);
    assert(q == Just(0));
    assert(q != Just(1));
    assert(q != Nothing);

    assert(Nothing != Just(1));

    return 0;
}

