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
#include <enumap.hpp>

#include <yats.hpp>
using namespace yats;


struct table : more::enumap<table>
{
    enumap_init(table,2);
    enumap_entry(hello, 0);
    enumap_entry(world, 1);
};


Context(enumap_class_test)
{
    Test(eval_string)
    {
        Assert(table::eval("hello"), is_equal_to(0));
        Assert(table::eval("world"), is_equal_to(1));
    }

    Test(eval_num)
    {
        Assert(table::eval(0), is_equal_to(std::string("hello")));
        Assert(table::eval(1), is_equal_to(std::string("world")));
    }

    Test(eval_enum)
    {
        Assert(static_cast<int>(table::hello), is_equal_to(0));
        Assert(static_cast<int>(table::world), is_equal_to(1));
    }
    
    Test(eval_get_value)
    {
        Assert(table::get<0>(), is_equal_to(std::string("hello")));
        Assert(table::get<1>(), is_equal_to(std::string("world")));
    }

    Test(direct_iterator)
    {
        std::cout << "\ndirect map:\n";
        auto it = table::direct.begin();
        for(; it != table::direct.end(); ++it)
        {
            std::cout << "   <" << it->first << "," << it->second << ">\n";
        }
    }

    Test(reverse_iterator)
    {
        std::cout << "\nreverse map:\n";
        auto ot = table::reverse.begin();
        for(; ot != table::reverse.end(); ++ot)
        {
            std::cout << "   <" << ot->first << "," << ot->second << ">\n";
        }
    }

    Test(has_method)
    {
        Assert(table::has("hello"), is_true());
        Assert(table::has(0), is_true());
        Assert(table::has(42), is_false());
    }

}

int
main(int argc, char *argv[])
{
    return yats::run();
}
