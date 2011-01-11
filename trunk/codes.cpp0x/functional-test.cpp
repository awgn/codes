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
#include <functional>
#include <memory>

#include <functional.hpp>
#include <yats.hpp>

using namespace yats;
using namespace std::placeholders;

Context(functional_tests)
{
    Test(logical_xor)
    {
        Assert(more::logical_xor(0,0) , is_equal_to(0));
        Assert(more::logical_xor(1,0) , is_equal_to(1));
        Assert(more::logical_xor(0,1) , is_equal_to(1));
        Assert(more::logical_xor(1,42), is_equal_to(0));
    }

    Test(chop)
    {
        more::chop<int> chopper(0,2);
        Assert(chopper(-1) , is_equal_to(0));
        Assert(chopper(1)  , is_equal_to(1));
        Assert(chopper(3)  , is_equal_to(2));
    }

    Test(flipflop)
    {
        more::flipflop<int> ff( (std::logical_not<int>()) );

        Assert( ff(1) , is_equal_to(0)); 
        Assert( ff(1) , is_equal_to(0)); 
        Assert( ff(0) , is_equal_to(1)); 
        Assert( ff(1) , is_equal_to(1)); 
        Assert( ff(1) , is_equal_to(1)); 
        Assert( ff(0) , is_equal_to(0)); 
        Assert( ff(1) , is_equal_to(0)); 
        Assert( ff(1) , is_equal_to(0)); 
    }

    Test(flipflop2)
    {
        more::flipflop2<int> ff2( std::bind(std::equal_to<int>(), _1, 0), 
                                  std::bind(std::equal_to<int>(), _1, 3) );    

        Assert( ff2(-2) , is_equal_to(0));
        Assert( ff2(-1) , is_equal_to(0));
        Assert( ff2(0)  , is_equal_to(1));
        Assert( ff2(1)  , is_equal_to(1));
        Assert( ff2(2)  , is_equal_to(1));
        Assert( ff2(3)  , is_equal_to(0));
        Assert( ff2(4)  , is_equal_to(0));
    }

    Test(norm)
    {
        more::norm<unsigned int> dist;

        unsigned int a = 10;
        unsigned int b = 52;

        Assert( dist(a,b) , is_equal_to(42));
        Assert( dist(b,a) , is_equal_to(42));
    }


    Test(identity)
    {
        Assert( more::identity<int>()(42), is_equal_to(42) );
    }                                                                    

    Test(select)
    {
        auto n = std::make_pair(11,42);                  

        auto p1 = more::select1st< std::pair<int,int> >(); 
        auto p2 = more::select2nd< std::pair<int,int> >();

        Assert( p1(n), is_equal_to(11) );
        Assert( p2(n), is_equal_to(42) );
    }

    Test(get_th)
    {
        auto p = std::make_tuple(10, 'c', std::string("hello"));

        auto t0 = more::get_th< 0, decltype(p)>();
        auto t1 = more::get_th< 1, decltype(p)>();
        auto t2 = more::get_th< 2, decltype(p)>();

        Assert( t0(p), is_equal_to(10) );
        Assert( t1(p), is_equal_to('c') );
        Assert( t2(p), is_equal_to(std::string("hello")) );
    }

    struct test
    {
        int value;
        test(int n)
        : value(n)
        {}
    
        void hello() { std::cout << "Hello World! (" << value << ")" << std::endl; }
    };

    Test(call_if)
    {
        std::vector<test *> vec = { new test(1), 0, 0, new test(2) };

        using namespace std::placeholders;
        std::for_each(vec.begin(), vec.end(), 
                  std::bind( more::call_if(std::mem_fn(&test::hello)), _1, _1));
    }

}
 
int
main(int argc, char *argv[])
{
    return yats::run();
}
 
