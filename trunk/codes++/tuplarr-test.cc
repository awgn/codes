/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <tuplarr.hh>

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std::tr1;

int
main(int argc, char *argv[])
{
    std::vector<double> vec;

    std::tr1::tuple<int,double> t = make_tuple(0,1);
    std::tr1::array<int,2> a = {2,3};

    more::container_backinsert_tuplarr(vec, t);
    more::container_backinsert_tuplarr(vec, a);

    std::copy(vec.begin(), vec.end(), std::ostream_iterator<double>(std::cout, " "));
    std::cout << std::endl;

    return 0;
}
 
