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
#include <functional>
#include <memory>

#include <functional.hpp>

using namespace std::placeholders;

struct test
{
    int value;
    test(int n)
    : value(n)
    {}

    void hello() { std::cout << "Hello World! (" << value << ")" << std::endl; }
};

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

    // norm

    unsigned int a = 10;
    unsigned int b = 52;
    
    more::norm<unsigned int> dist;

    std::cout << "norm: |10-52| = " << dist(a,b) << std::endl;
    std::cout << "norm: |52-10| = " << dist(b,a) << std::endl;

    // identity

    std::cout << "idenity<int>(42) = " << more::identity<int>()(42) << std::endl;

    // select1st, select2nd

    std::cout << "select1st<pair<int,int> >(make_pair(11,42)) = " << more::select1st< std::pair<int,int> >()(std::make_pair(11,42)) << std::endl;
    std::cout << "select2nd<pair<int,int> >(make_pair(11,42)) = " << more::select2nd< std::pair<int,int> >()(std::make_pair(11,42)) << std::endl;

    // call_if test:

    std::vector<test *> vec;

    vec.push_back(new test(1));
    vec.push_back(0);
    vec.push_back(0);
    vec.push_back(new test(2));

    using namespace std::placeholders;
    std::for_each(vec.begin(), vec.end(), 
                  std::bind( more::call_if(std::mem_fn(&test::hello)), _1, _1));

    return 0;
}
 
