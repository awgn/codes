/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <any.hpp>
#include <iostream>
#include <sstream>

#include <yats.hpp>

using namespace yats;

Context(any_class_test)
{  
    more::any a(1);
    const more::any b(std::string("hello world!"));
    
    Test(default_constructible)
    {
        more::any c;
    }

    Test(copy_constructible)
    {
        more::any copy(a);
        Assert( more::any_cast<int>(a), is_equal_to( more::any_cast<int>(copy)) ); 
    }

    Test(empty)
    {
        Assert(a.empty(), is_false());
        Assert(more::any().empty(), is_true());
    }
    
    Test(any_cast)
    {
        Assert(more::any_cast<int>(a), is_equal_to(1));
        Assert_Throw(more::any_cast<int>(b));
    }

    Test(any_out)
    {
        more::any_out a_out(10);
        more::any_out b_out(std::string("Hello world!") );

        std::ostringstream out;

        out << a_out;
        Assert(out.str(), is_equal_to(std::string("10")));
        out.str("");

        out << b_out;
        Assert(out.str(), is_equal_to(std::string("Hello world!")));
    }    

}
 

int
main(int argc, char *argv[])
{
    return yats::run();
}
