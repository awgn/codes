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

struct test {};

struct print_on 
{
    template <typename T> 
    void operator()(T &e)
    {
        std::cout << "->" << e << "<-" << std::endl;
    }
};

using namespace std::tr1::placeholders;

int
main(int, char *[])
{
    std::vector<double> vec;

    std::tr1::tuple<int,double,int> t = std::tr1::make_tuple(0,1.0,2);
    std::tr1::array<int,2> a = {{ 3, 4 }};

    more::tuplarr::copy(t, std::back_inserter(vec));
    more::tuplarr::copy(a, std::back_inserter(vec));

    std::cout << "vec: ";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<double>(std::cout, " "));
    std::cout << std::endl;

    std::cout << t << std::endl;
    std::cout << a << std::endl;

    std::tr1::tuple<> nt;
    std::tr1::array<int,0> na;

    std::cout << nt << std::endl;
    std::cout << na << std::endl;

    // for_each...

    std::cout << "for_each: " << std::endl;

    more::tuplarr::for_each(t, print_on());

    // count

    std::cout << "count: " << 
        more::tuplarr::count(t, 1) 
        << std::endl;


    // count_if

    std::cout << "count_if: " << 
        more::tuplarr::count_if(t, std::tr1::bind(std::greater<double>(), _1, 0)) 
        << std::endl;

    return 0;
}
 
