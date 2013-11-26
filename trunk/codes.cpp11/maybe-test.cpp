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
#include <algorithm>
#include <vector>

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
        Assert(q.fromJust() == 0);
        Assert(q == Just(0));
        Assert(q != Just(1));
        Assert(q != Nothing);

        Assert(Nothing != Just(1));
    }

    Test(test_stream)
    {
        Maybe<int> x = Nothing;

        std::cout << x << std::endl;
        std::cout << Nothing << std::endl;
        std::cout << Just(1) << std::endl;
    }

    Test(less)
    {
        Maybe<int> n = Nothing;

        Assert(Nothing < Nothing, is_false());
        Assert(Nothing < n, is_false());
        Assert(n < Nothing, is_false());

        Assert(Nothing < Just(1), is_true());
        Assert(Just(1) < Nothing, is_false());

        Assert(n < Just(1), is_true());
        Assert(Just(1) < n, is_false());
    }

    Test(greater)
    {
        Maybe<int> n = Nothing;

        Assert(Nothing > Nothing, is_false());
        Assert(Nothing > n, is_false());
        Assert(n > Nothing, is_false());

        Assert(Nothing > Just(1), is_false());
        Assert(Just(1) > Nothing, is_true());

        Assert(n > Just(1), is_false());
        Assert(Just(1) > n, is_true());
    }

    Test(vector_of_maybe)
    {
        Maybe<int> n;

        std::vector<Maybe<int>> xs = { n, Just(42), Just(0), Nothing };
        std::sort(xs.begin(), xs.end());

        std::vector<Maybe<int>> out = { Nothing, Nothing, Just(0), Just(42) };

        Assert(std::equal(xs.begin(), xs.end(), out.begin()) );

        std::cout << "-> ";
        for(auto & x : xs)
        {
            std::cout << x << ' ';
        }
        std::cout << std::endl;
    }

    Test(fmap)
    {
        auto x = Just (41);

        auto y = x.fmap([](int n) { return n+1; });

        Maybe<int> q = Nothing;
        auto z = q.fmap([](int n) { return n+1; });

        std::cout << y << std::endl;
        std::cout << z << std::endl;
    }
}

int
main(int argc, char *argv[])
{

    return yats::run(argc, argv);
}

