/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#include "yats.hpp"
#include "byte_string.hpp"

using namespace yats;
using namespace more;

Context(ByteString)
{
    Test(ctor)
    {
        byte_string<8> x;
        Assert( x.empty(), is_true());
        Assert( static_cast<bool>(x), is_false());

        Assert( x.length(), is_equal_to(0));
        Assert( x.size  (), is_equal_to(0));
        Assert( x.capacity(), is_equal_to(7));
    }

    Test(ctor2)
    {
        byte_string<8> x("hello");
        Assert( x.empty(), is_false());
        Assert( static_cast<bool>(x), is_true());
        Assert( x.length(), is_equal_to(5));
        Assert( x.size  (), is_equal_to(5));
        Assert( x.capacity(), is_equal_to(7));
    }
    
    Test(ctor3)
    {
        byte_string<8> x(std::string("hello"));
        Assert( x.empty(), is_false());
    }

    Test(copyctor)
    {
        byte_string<8> x("hello");
        byte_string<6> y(x);

        AssertThrow( byte_string<5> z(y) );
        Assert(x == y);
    }

    Test(assign)
    {
        byte_string<8> x("hello");
        byte_string<6> y;
        byte_string<4> z;
                             
        AssertNothrow  ( y = x );
        AssertThrow    ( z = x );

        Assert(x == y);
    }

    Test(operEq)
    {
        byte_string<8> x("hello");
        byte_string<6> y(x);

        Assert(x == x);
        Assert(x == y);
        Assert(y == x);
        
        Assert(x == "hello");
        Assert("hello" == x);
        Assert(x == std::string("hello"));
        Assert(std::string("hello") == x);
    }

    Test(operNotEq)
    {
        byte_string<8> x("hello");
        byte_string<6> y(x);

        Assert(!(x != x));
        Assert(!(x != y));
        Assert(!(y != x));
        
        Assert(!(x != "hello"));
        Assert(!("hello" != x));
        Assert(!(x != std::string("hello")));
        Assert(!(std::string("hello") != x));
    }
    
    Test(operLess)
    {
        byte_string<8> x("hello");
        byte_string<6> y("world");

        Assert( x < y );
    }
}


int
main(int argc, char*  argv[])
{
    return yats::run(argc, argv);
}

