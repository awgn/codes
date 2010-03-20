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
#include <algorithm.hh>

using namespace std::placeholders;

int
main(int argc, char *argv[])
{
    // logical xor
    std::cout << "logical_xor: " << more::logical_xor(0,0) << std::endl;
    std::cout << "logical_xor: " << more::logical_xor(1,0) << std::endl;
    std::cout << "logical_xor: " << more::logical_xor(0,1) << std::endl;
    std::cout << "logical_xor: " << more::logical_xor(1,42) << std::endl;

    // chop
    more::chop<int> chopper(0,2);
    std::cout << "chop: " << chopper(-1) << std::endl;
    std::cout << "chop: " << chopper(1) << std::endl;
    std::cout << "chop: " << chopper(3) << std::endl;

    // flipflop
    more::flipflop<int> ff1( (std::logical_not<int>()) );

    std::cout << "flipflop: " << ff1(1) << std::endl; 
    std::cout << "flipflop: " << ff1(1) << std::endl; 
    std::cout << "flipflop: " << ff1(0) << std::endl; 
    std::cout << "flipflop: " << ff1(1) << std::endl; 
    std::cout << "flipflop: " << ff1(1) << std::endl; 
    std::cout << "flipflop: " << ff1(0) << std::endl; 
    std::cout << "flipflop: " << ff1(1) << std::endl; 
    std::cout << "flipflop: " << ff1(1) << std::endl; 

    // flipflop2
    more::flipflop2<int> ff2( std::bind(std::equal_to<int>(),_1, 0), std::bind(std::equal_to<int>(), _1, 3) );    

    std::cout << "flipflop2: " << -2 << " " << ff2(-2) << std::endl;
    std::cout << "flipflop2: " << -1 << " " << ff2(-1) << std::endl;
    std::cout << "flipflop2: " << 0  << " " << ff2(0)  << std::endl;
    std::cout << "flipflop2: " << 1  << " " << ff2(1)  << std::endl;
    std::cout << "flipflop2: " << 2  << " " << ff2(2)  << std::endl;
    std::cout << "flipflop2: " << 3  << " " << ff2(3)  << std::endl;
    std::cout << "flipflop2: " << 4  << " " << ff2(4)  << std::endl;

    // levenshtein distance
    //

    std::string a("hello");
    std::string b("Hello");
    std::string c("helol");

    std::cout << a << ':' << c << " -> " << more::levenshtein_distance(a.begin(), a.end(), c.begin(), c.end(), std::equal_to<char>()) << std::endl;
    std::cout << c << ':' << a << " -> " << more::levenshtein_distance(c.begin(), c.end(), a.begin(), a.end(), std::equal_to<char>()) << std::endl;
    std::cout << a << ':' << a << " -> " << more::levenshtein_distance(a.begin(), a.end(), a.begin(), a.end(), std::equal_to<char>()) << std::endl;
    std::cout << a << ':' << b << " -> " << more::levenshtein_distance(a.begin(), a.end(), b.begin(), b.end()) << std::endl;
    std::cout << a << ':' << b << " -> " << more::levenshtein_distance(a,b) << " (std::string)" << std::endl;
        
    return 0;
}
 
