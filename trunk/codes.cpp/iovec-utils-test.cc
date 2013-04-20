/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iovec-utils.hh>

#include <iostream>
#include <algorithm>
#include <iterator>

#include <string>
#include <vector>
#include <deque>
#include <list>

int
main(int, char *[])
{
    // ------- deque (int) 

    std::deque<int> abc;

    abc.push_back(0);
    abc.push_back(1);
    abc.push_back(2);
    abc.push_back(3);

    abc.push_front(-1);
    abc.push_front(-2);
    abc.push_front(-3);
    abc.push_front(-4);

    std::vector<iovec> iov = more::get_iovec(abc.begin(), abc.end());
    std::copy(iov.begin(), iov.end(), std::ostream_iterator<iovec>(std::cout, "-"));
    std::cout << std::endl;

    std::copy(abc.begin(), abc.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    // ------- deque (hello world) 

    std::deque<char> hello_world;

    std::string hello(" ,olleH");
    std::string world("World!");

    std::copy(hello.begin(), hello.end(), std::front_inserter(hello_world));
    std::copy(world.begin(), world.end(), std::back_inserter (hello_world));
    hello_world.push_back('\n');

    std::vector<iovec> iov2 = more::get_iovec(hello_world.begin(), hello_world.end());

    std::copy(iov2.begin(), iov2.end(), std::ostream_iterator<iovec>(std::cout, "-"));
    std::cout << std::endl;

    ::writev(1, &iov2.front(), iov2.size());

    // ----- list

    std::list<char> l;

    l.push_back('a');
    l.push_back('b');
    l.push_back('c');
    l.push_back('\n');

    std::vector<iovec> liov = more::get_iovec(l.begin(), l.end());

    std::copy(liov.begin(), liov.end(), std::ostream_iterator<iovec>(std::cout, "-"));
    std::cout << std::endl;

    ::writev(1, &liov.front(), liov.size());

    // ----- vector<std::string>

    std::vector<std::string> vec;

    vec.push_back("Hello,");
    vec.push_back(" world!\n");

    std::vector<iovec> viov = more::get_iovec(vec.begin(), vec.end());

    std::copy(viov.begin(), viov.end(), std::ostream_iterator<iovec>(std::cout, "-"));
    std::cout << std::endl;

    ::writev(1, &viov.front(), viov.size());

    // ----- vector<std::string>

    std::vector<unsigned int> vec3(3, 0x41414141);
    vec3.push_back(0x0a000000);

    std::vector<iovec> viov3 = more::get_iovec(vec3.begin(), vec3.end());
    std::copy(viov3.begin(), viov3.end(), std::ostream_iterator<iovec>(std::cout, "-"));
    std::cout << std::endl;
    ::writev(1, &viov3.front(), viov3.size());

    // empty test
    //

    std::vector<char> vec4;
    std::vector<iovec> viov4 = more::get_iovec(vec4.begin(), vec4.end());
    std::copy(viov4.begin(), viov4.end(), std::ostream_iterator<iovec>(std::cout, "-"));
    
   return 0;
}

