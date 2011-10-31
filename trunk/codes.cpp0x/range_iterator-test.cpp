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
#include <range_iterator.hpp>

#include <vector>
#include <list>

int
main(int argc, char *argv[])
{
    char buffer[4];
    const char *const_buffer = buffer;

    std::vector<char> vec(4);

    auto c1 = more::range_iterator(buffer, buffer+4);
    // auto c2 = more::range_iterator(const_buffer, const_buffer+4);        // error: ok
    auto c3 = more::range_const_iterator(buffer, buffer+4);                 // ok, cctor of CV-qualified type
    auto c4 = more::range_const_iterator(const_buffer, const_buffer+4);     // ok, cctor of CV-qualified type
    
    auto c5 = more::range_iterator(vec.begin(), vec.end());
    // auto c6 = more::range_iterator(vec.cbegin(), vec.cend());               // error: ok

    auto c7 = more::range_const_iterator(vec.begin(), vec.end());
    auto c8 = more::range_const_iterator(vec.cbegin(), vec.cend());        

    * c1++ = 'a';
    * c1++ = 'b';

    std::cout << "c1 == c1: " << (c1 == c1) << std::endl;
    std::cout << "c1 == c3: " << (c1 == c3) << std::endl;

    std::cout << "size: " << c1.size() << " slot consumed." << std::endl;
    std::cout << "size: " << c1.capacity() << " slot left." << std::endl;

    std::cout << "*c3++ = " << * c3++ << std::endl;
    std::cout << "*c3++ = " << * c3++ << std::endl;

#ifdef __clang__    
    std::vector<int> vec2;
    vec2.push_back(1);
    vec2.push_back(2);
    vec2.push_back(3);
#else
    std::vector<int> vec2 = {1,2,3};
#endif

    auto il = more::range_iterator(vec2.begin(), vec2.end());
    
    std::copy(il, il.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "done." << std::endl;
    return 0;
}
 
