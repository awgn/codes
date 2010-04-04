/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <static_allocator.h>

#include <iostream>
#include <iterator>
#include <algorithm>
#include <vector>
#include <list>

int
main(int argc, char *argv[])
{
    /// vector test
    {
        int buffer[4];

        std::vector<int, more::static_allocator<int> > abc( more::static_allocator<int>(buffer,16) );
        abc.reserve(4);

        abc.push_back(1);
        abc.push_back(2);
        abc.push_back(3);

        std::make_heap(abc.begin(), abc.end());

        std::cout << "max:" << abc[0] << std::endl;

        std::pop_heap(abc.begin(), abc.end());

        abc.pop_back();

        abc.push_back(5);
        std::push_heap(abc.begin(), abc.end());

        std::cout << "max:" << abc[0] << std::endl;


        std::copy(abc.begin(), abc.end(), std::ostream_iterator<int>(std::cout," ")), std::cout << std::endl;
    }

    /// list test
    {
        char buffer[128];

        std::list<int, more::static_allocator<int> > abc( more::static_allocator<int>(buffer,128) );

        abc.push_back(1);
        abc.push_back(2);

        std::copy(abc.begin(), abc.end(), std::ostream_iterator<int>(std::cout, " ")), std::cout << std::endl;
    }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
    /// vector with initializer_list<int>
    {
        int buffer[4];
        std::vector<int, more::static_allocator<int> > abc( {1,2,3,4}, more::static_allocator<int>(buffer,16) );
        
        std::copy(abc.begin(), abc.end(), std::ostream_iterator<int>(std::cout, " ")), std::cout << std::endl;
    }
#endif 

    return 0;
}
 
