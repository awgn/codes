/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

#include <tuplarr.hh>

using namespace std::tr1;

struct test {};

int
main(int argc, char *argv[])
{
    std::vector<double> vec;

    std::tr1::tuple<int,double> t = make_tuple(0,1);
    std::tr1::array<int,2> a = { 2, 3 };

    more::container_backinsert_tuplarr(vec, t);
    more::container_backinsert_tuplarr(vec, a);

    std::cout << "vec: ";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<double>(std::cout, " "));
    std::cout << std::endl;

    std::cout << t << std::endl;
    std::cout << a << std::endl;

    std::tr1::tuple<> nt;
    std::tr1::array<int,0> na;

    std::cout << nt << std::endl;
    std::cout << na << std::endl;

    return 0;
}
 
