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
    Test(apply)
    {
        auto add = [](int a, int b) { return a+b; };

        std::tuple<int, int> tup{40,2};

        Assert(more::tuple_apply(add, tup), is_equal_to(42));
    }

    Test(apply_const)
    {
        auto add = [](int a, int b) { return a+b; };

        std::tuple<int, int> const tup{40,2};

        Assert(more::tuple_apply(add, tup), is_equal_to(42));

    }

    Test(apply_forward)
    {
        auto mistery = [](int &a, int b) -> int {
                a = 10;
            return a+b; };

        int a = 40;

        Assert(more::tuple_apply(mistery, std::forward_as_tuple(a,2)), is_equal_to(12));
        Assert(a, is_equal_to(10));
    }


    template <typename T>
    void test(T &v)
    {
        long int sum = 0;
        more::tuple_for_each(v, [&](typename std::tuple_element<0, T>::type n) {
                                sum += n;
                             });
    }

    Test(for_each)
    {
        std::tuple<int,int,int> v {1,2,3};
        std::tuple<long, long> q {1,2};

        test(v);
        test(q);
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

    Test(map)
    {
        auto t = more::tuple_map(strlen, std::make_tuple("hello", "world"));

        size_t len = 0;

        more::tuple_for_each(t, [&len](size_t n) { len+=n; });

        Assert(len, is_equal_to(size_t(10)));
    }

    Test(map_const)
    {
        auto const t = std::make_tuple("hello", "world");

        auto ts = more::tuple_map(strlen, t);

        size_t len = 0;

        more::tuple_for_each(ts, [&len](size_t n) { len+=n; });

        Assert(len, is_equal_to(size_t(10)));
    }
}


int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}


