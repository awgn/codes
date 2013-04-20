/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */


#include <algorithm.hh>

#include <tr1/functional>
#include <iostream>
#include <iterator>
#include <functional>


using namespace std::tr1::placeholders;

int
main(int, char *[])
{
    // the strange case of copy_if...

    std::vector<int> vec(10);

    more::iota(vec.begin(), vec.end(), 0);

    std::copy(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, " "));
    std::cout << std::endl;

    more::copy_if(vec.begin(), vec.end(), std::ostream_iterator<int>(std::cout, " "), 
                 std::tr1::bind(std::greater<int>(), 5, _1) );
    std::cout << std::endl;

    // levenshtein distance
    //

    std::string a("hello");
    std::string b("Hello");
    std::string c("helol");
    std::string d("helo");

    std::cout << a << ':' << c << " -> " << more::levenshtein_distance(a.begin(), a.end(), c.begin(), c.end(), std::not_equal_to<char>()) << std::endl;
    std::cout << c << ':' << a << " -> " << more::levenshtein_distance(c.begin(), c.end(), a.begin(), a.end(), std::not_equal_to<char>()) << std::endl;
    std::cout << a << ':' << a << " -> " << more::levenshtein_distance(a.begin(), a.end(), a.begin(), a.end(), std::not_equal_to<char>()) << std::endl;
    std::cout << a << ':' << b << " -> " << more::levenshtein_distance(a.begin(), a.end(), b.begin(), b.end()) << std::endl;
    std::cout << a << ':' << d << " -> " << more::levenshtein_distance(a.begin(), a.end(), d.begin(), d.end()) << std::endl;
    std::cout << a << ':' << b << " -> " << more::levenshtein_distance(a,b) << " (std::string)" << std::endl;
        
    return 0;
}
 
