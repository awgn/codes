/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <yats.hpp>

using namespace yats;

Context(yet_another_test_suite)
{
    Test(test_0)
    {
        Assert(std::vector<int>().empty(), is_false());
    }
    Test(test_0_ok)
    {
        Assert(std::vector<int>().empty(), is_true());
    }

    Test(test_1)
    {
        Assert(!std::vector<int>().empty(), is_true());
    }
    Test(test_1_ok)
    {
        Assert(!std::vector<int>().empty(), is_false());
    }

    Test(test_2)
    {
        Assert(1, is_greater(2));
    }
    Test(test_2_ok)
    {
        Assert(2, is_greater(1));
    }

    Test(test_3)
    {
        Assert(1, is_greater_equal(2));
    }
    Test(test_3_ok)
    {
        Assert(2, is_greater_equal(2));
    }

    Test(test_4)
    {
        Assert(2, is_less(2));
    }
    Test(test_4_ok)
    {
        Assert(1, is_less(2));
    }

    Test(test_5)
    {
        Assert(2, is_less_equal(1));
    }
    Test(test_5_ok)
    {
        Assert(0, is_less_equal(1));
    }

    Test(test_6)
    {
        Assert(42, is_equal_to(39));
    }
    Test(test_6_ok)
    {
        Assert(42, is_equal_to(42));
    }

    Test(test_7)
    {
        Assert(42, is_not_equal_to(42));
    }
    Test(test_7_ok)
    {
        Assert(42, is_not_equal_to(39));
    }
    
    ////////////////// exceptions

    void fun_nothrow()
    {}

    template <typename Tp>
    void fun_throw(const Tp &n)
    {
        throw n;
    }

    Test(test_8)
    {
        Assert_nothrow(fun_throw(std::runtime_error("error")));
    }
    Test(test_9)
    {
        Assert_nothrow(fun_throw(0));
    }
    Test(test_9_ok)
    {
        Assert_nothrow(fun_nothrow());
    }
 
    Test(test_10)
    {
        Assert_throw(fun_nothrow());
    }
    Test(test_10_ok)
    {
        Assert_throw(fun_throw(0));
    }

    Test(test_11)
    {
        Assert_throw_type(fun_throw(std::runtime_error("error")), std::logic_error);
    }
    Test(test_12)
    {
        Assert_throw_type(fun_nothrow(), std::logic_error);
    }
    Test(test_12_ok)
    {
        Assert_throw_type(fun_throw(std::logic_error("ok")), std::logic_error);
    }
}

int
main(int argc, char *argv[])
{
    return test::run();
}
 
