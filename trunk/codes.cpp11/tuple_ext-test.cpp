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
#include <tuple_ext.hpp>

#include <yats.hpp>

using namespace yats;


Context(tuple_extended)
{

    Test(call)
    {
        auto somma = [](int a, int b) { return a+b; };

        std::tuple<int, int> tup{40,2};

        Assert(more::tuple_call(somma, tup), is_equal_to(42));
    }

    Test(call_const)
    {
        auto somma = [](int a, int b) { return a+b; };

        std::tuple<int, int> const tup{40,2};

        Assert(more::tuple_call(somma, tup), is_equal_to(42));

    }

    Test(call_forward)
    {
        auto somma = [](int a, int b) { return a+b; };

        int a = 40, b = 2;

        Assert(more::tuple_call(somma, std::forward_as_tuple(a,b)), is_equal_to(42));
    }


    Test(for_each)
    {
        std::tuple<int,int,int> v {1,2,3};
        int sum = 0;

        more::tuple_for_each(v, [&](int n) {
                              sum += n;
                             });

        Assert(sum, is_equal_to(6));
    }
    
    Test(for_each_const)
    {
        std::tuple<int,int,int> const v {1,2,3};
        int sum = 0;

        more::tuple_for_each(v, [&](int n) {
                              sum += n;
                             });

        Assert(sum, is_equal_to(6));
    }
    Test(for_each_forward)
    {
        int a = 1, b = 2, c = 3;
        int sum = 0;

        more::tuple_for_each(std::forward_as_tuple(a,b,c), [&](int n) {
                              sum += n;
                             });

        Assert(sum, is_equal_to(6));
    }
}


int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 

