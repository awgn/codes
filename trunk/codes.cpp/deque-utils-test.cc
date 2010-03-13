/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <deque-utils.hh>

#include <iostream>
#include <algorithm>
#include <iterator>

int
main(int argc, char *argv[])
{
    std::deque<int> abc;

    abc.push_back(0);
    abc.push_back(1);
    abc.push_back(2);
    abc.push_back(3);

    abc.push_front(-1);
    abc.push_front(-2);
    abc.push_front(-3);
    abc.push_front(-4);

    std::copy(abc.begin(), abc.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    std::vector<iovec> iov = more::deque_iovec(abc);

    std::vector<iovec>::iterator it = iov.begin();
    for(; it != iov.end(); ++it)
    {
        std::cout << "iovec<" << it->iov_base << "," << it->iov_len << ">" << std::endl;
    }

    // ------- hello world ----------

    std::deque<char> hello_world;

    std::string hello(" ,olleH");
    std::string world("World!");

    std::copy(hello.begin(), hello.end(), std::front_inserter(hello_world));
    std::copy(world.begin(), world.end(), std::back_inserter (hello_world));
    hello_world.push_back('\n');

    std::vector<iovec> iov2 = more::deque_iovec(hello_world);
    ::writev(1, &iov2.front(), iov2.size());

    return 0;
}
 
