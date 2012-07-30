/* $Id: iovec-utils-test.cc 480 2010-03-21 20:26:21Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */


#include <iostream>
#include <algorithm>
#include <iterator>

#include <cstring>
#include <string>
#include <vector>
#include <deque>
#include <list>

#include <iovec.hpp>
#include <yats.hpp>
using namespace yats;


Context(more_iovec_test)
{
    Test(get_iovec_on_deque)
    {
#ifdef __clang__
        std::deque<int> abc;
        abc.push_back(0);
        abc.push_back(1);
        abc.push_back(2);
        abc.push_back(3);
#else
        std::deque<int> abc = { 0, 1, 2, 3 };
#endif
        abc.push_front(-1);
        abc.push_front(-2);
        abc.push_front(-3);
        abc.push_front(-4);

        std::vector<iovec> iov = more::get_iovec(abc.begin(), abc.end());
        
        Assert(iov.size(),     is_equal_to(2UL));
        Assert(iov[0].iov_len, is_equal_to(sizeof(int)*4));
        Assert(iov[1].iov_len, is_equal_to(sizeof(int)*4));
    }

    Test(get_iovec_on_deque_char)
    {
        std::deque<char> hello_world;

        std::string hello(" ,olleH");
        std::string world("World!");

        std::copy(hello.begin(), hello.end(), std::front_inserter(hello_world));
        std::copy(world.begin(), world.end(), std::back_inserter (hello_world));
        hello_world.push_back('\n');

        std::vector<iovec> iov = more::get_iovec(hello_world.begin(), hello_world.end());
        
        const char HelloWorld[] = "Hello, World!\n";
        Assert( ( std::equal( more::get_iovec_iterator<char>(iov),
                              more::get_iovec_iterator<char>(), 
                              HelloWorld) ) , is_true());

    }

    Test(get_iovec_on_list)
    {
        std::list<char> l;

        l.push_back('a');
        l.push_back('b');
        l.push_back('c');

        std::vector<iovec> iov = more::get_iovec(l.begin(), l.end());

        const char abc[] =  "abc";
        Assert( ( std::equal( more::get_iovec_iterator<char>(iov),
                              more::get_iovec_iterator<char>(), 
                              abc) ) , is_true());
    }

    Test(get_iovec_on_vector_of_string)
    { 
        std::vector<std::string> vec;

        vec.push_back("Hello,");
        vec.push_back(" World!");

        std::vector<iovec> iov = more::get_iovec(vec.begin(), vec.end());

        Assert(iov.size(),     is_equal_to(2UL));
        Assert(iov[0].iov_len, is_equal_to(6UL));
        Assert(iov[1].iov_len, is_equal_to(7UL));

        Assert(memcmp(iov[0].iov_base, "Hello,",  6), is_equal_to(0)); 
        Assert(memcmp(iov[1].iov_base, " World!", 7), is_equal_to(0)); 
    }
                         
    Test(get_iovec_empty_container)
    {
        std::vector<char> vec;
        std::vector<iovec> iov = more::get_iovec(vec.begin(), vec.end());
        Assert( iov.size(), is_equal_to(0UL));
    }
}

int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 

