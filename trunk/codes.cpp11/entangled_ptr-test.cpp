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
#include <yats.hpp>

using namespace yats;

Context(entangled_ptr_tests)
{
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

    test x;
    test y;
    test z;

    more::entangled_ptr<test> p; 
    more::entangled_ptr<test> q; 
    more::entangled_ptr<test> w; 

    Test(pointer_assignable)
    {
        x.value = 42;
        p = x.entangled_from_this();
    }

    Test(check_address_get)
    {
        Assert(p.get(), is_equal_to(&x));
    }
        
    Test(operator_arrow)
    {
        Assert(p->value, is_equal_to(x.value));
    }

    Test(pointed_moveable)
    {
        y = std::move(x);
        Assert(p.get(), is_equal_to(&y));
        Assert(p->value, is_equal_to(y.value));
    }

    Test(pointer_assignable_2)
    {
        q = y.entangled_from_this();
        Assert(q.get(), is_equal_to(&y));
        Assert(q->value, is_equal_to(y.value));
    }

    Test(two_pointers_one_object)
    {
        Assert(p.get(), is_equal_to(q.get()));
    }

    Test(moving_object_double_referenced)
    {
        z = std::move(y);

        Assert(p.get(), is_equal_to(&z));
        Assert(q.get(), is_equal_to(&z));
    }

    Test(copy_constructible)
    {
        more::entangled_ptr<test> y(p);
        Assert(y.get(), is_equal_to(&z));
    }

    Test(use_count)
    {
        Assert(p.use_count(), is_equal_to(2UL));
        Assert(q.use_count(), is_equal_to(2UL));
    }

    Test(bad)
    {
        test error;
        error.value = 11;
        w = error.entangled_from_this();
    }

    Test(dangling_pointer)
    {
        Assert(w.get(), is_equal_to(static_cast<test *>(nullptr)));
        AssertThrow(*w);
    }
}

  int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}


