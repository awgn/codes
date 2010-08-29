/* $Id: algorithm-test.cc 500 2010-03-28 12:15:09Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */


#include <algorithm.hpp>

#include <iostream>
#include <iterator>
#include <functional>

using namespace std::placeholders;

int
main(int argc, char *argv[])
{
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
 
