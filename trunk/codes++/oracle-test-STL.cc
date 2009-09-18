/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <oracle.hh>

#include <vector>
#include <algorithm>

struct hello : public more::oracle<hello>
{
    hello()
    {}

    hello(const std::string &name)
    : oracle_base(name)
    {}
};

oracle_std_swap(hello);


int
main(int argc, char *argv[])
{
    more::oracle_trace<hello>::enable();

    std::vector<hello> l;

    l.push_back(hello("hello"));
    l.push_back(hello());
    l.push_back(hello());

    l[2] = hello();

    std::sort(l.begin(), l.end());
    return 0;
}
 
