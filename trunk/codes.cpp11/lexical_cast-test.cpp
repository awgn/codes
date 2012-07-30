/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <lexical_cast.hpp>
#include <boost/lexical_cast.hpp>

#include <thread>
#include <iostream>

#include <yats.hpp>
using namespace yats;

Context(more_lexical_cast)
{
    Test(integer)
    {
        Assert( more::lexical_cast<int>("123"), is_equal_to(123) );
        AssertThrow( more::lexical_cast<int>("123A"));
    }

    Test(unsigned_integer)
    {
        Assert( more::lexical_cast<unsigned int>("123"), is_equal_to(123) );
    }

    Test(int_to_string)
    {
        Assert( more::lexical_cast<std::string>(123), is_equal_to(std::string("123")) );
        Assert( more::lexical_cast<std::string>(-123), is_equal_to(std::string("-123")) );
    }

    Test(double_to_string)
    {
        Assert( more::lexical_cast<std::string>(4.2), is_equal_to(std::string("4.2")) );
    }
}

int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}

