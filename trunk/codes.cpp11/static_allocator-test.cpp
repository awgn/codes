/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <static_allocator.hpp>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <list>

#include <yats.hpp>
using namespace yats;

Context(more_static_allocator)
{
    Test(vector_with_static_buffer)
    {
        int buffer[4];

        std::vector<int, more::static_allocator<int> > abc( more::static_allocator<int>(buffer,16) );
        abc.reserve(4);

        abc.push_back(1);
        abc.push_back(2);
        abc.push_back(3);

        std::make_heap(abc.begin(), abc.end());

        Assert( abc[0], is_equal_to(3));

        std::pop_heap(abc.begin(), abc.end());

        abc.pop_back();

        abc.push_back(5);
        std::push_heap(abc.begin(), abc.end());

        Assert( abc[0], is_equal_to(5));

        Assert( (std::equal(abc.begin(), abc.end(), (int *)buffer)), is_true() );
    }

    Test(list_with_static_buffer)
    {
        char buffer[128];

        std::list<int, more::static_allocator<int> > abc( more::static_allocator<int>(buffer,128) );

        abc.push_back(1);
        abc.push_back(2);

        int out[] = { 1, 2 };
        Assert( (std::equal(abc.begin(), abc.end(), out)), is_true() );
    }

    Test(vector_with_initializer_list)
    {
        int buffer[4];
#ifdef __clang__
        std::vector<int, more::static_allocator<int> > abc(more::static_allocator<int>(buffer,16));
        abc.push_back(1);
        abc.push_back(2);
        abc.push_back(3);
        abc.push_back(4);
#else
        std::vector<int, more::static_allocator<int> > abc({1,2,3,4}, more::static_allocator<int>(buffer,16) );
#endif

        int out[] = { 1, 2, 3, 4 };
        Assert( (std::equal(abc.begin(), abc.end(), out)), is_true() );
    }

}

int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
