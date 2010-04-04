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

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

int
main(int argc, char *argv[])
{
    int buffer[4];

    std::vector<int, more::static_allocator<int> > abc( more::static_allocator<int>(buffer,4) );
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

    ///

    std::copy(abc.begin(), abc.end(), std::ostream_iterator<int>(std::cout," ")), std::cout << std::endl;

    return 0;
}
 
