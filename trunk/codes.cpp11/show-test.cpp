/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <show.hpp> // !more

#include <vector>
#include <list>
#include <map>
#include <iostream>

struct test
{
    std::string value;
};

inline std::string
show(const test &t, label)
{
    return "test " + show(t.value, none);    
}

int 
main(int, char *[])
{
    std::cout << "--- basic (only values):\n\n";

    uint8_t uc = '\x42';

    const uint8_t ua[3] = {1, 2, 3};

    std::cout << show(uc, none) << std::endl; 
    std::cout << show(ua, none) << std::endl; 

    std::cout << show("hello world", none) << std::endl; 
    std::cout << show(std::string("hello world"), none) << std::endl; 

    std::cout << show((void *)0xdeadbeef, none) << std::endl; 

    auto p = std::make_pair("hello world", 42);

    std::cout << show(p,none) << std::endl; 
    
    std::vector<int> v1;

    v1.push_back(0);
    v1.push_back(1);
    v1.push_back(2);

    std::cout << show(v1,none) << std::endl;

    std::vector<char> v2;

    v2.push_back('a');
    v2.push_back('b');
    v2.push_back('c');

    std::cout << show(v2,none) << std::endl;

    std::list<std::pair<int,int> > l1;

    l1.push_back( std::make_pair(0,0) );
    l1.push_back( std::make_pair(1,1) );
    l1.push_back( std::make_pair(2,2) );

    std::cout << show(l1,none) << std::endl;

    std::map<std::string,int> m;

    m["hello"]=0;
    m["world"]=1;

    std::cout << show(m,none) << std::endl;

    std::cout << show(std::tuple<int,double>(1,1.1), none) << std::endl;
    
    std::array<int,3> ax = {{1, 2, 3}};
    
    std::cout << show(ax, none) << std::endl;

    std::cout << show(std::chrono::system_clock::now(), none ) << std::endl;
    std::cout << show(std::chrono::milliseconds(10), none) << std::endl;
    std::cout << show(test{"abc"}, none) << std::endl;

    int c_array[3] = {1,2,3};
    
    std::cout << "\n--- with typename:\n\n";

    auto c = std::make_tuple(std::vector<int>{1,2}, std::make_pair(1,2), std::chrono::seconds(1), std::make_tuple(1,2));
    
    std::cout << show(c, label("")) << std::endl;
    std::cout << show(c_array, label("")) << std::endl;
    
    std::cout << "\n--- with custom name:\n\n";
    
    std::cout << show(c_array, label("C_array")) << std::endl;
    std::cout << show(42, label("Integer:")) << std::endl;
    
    std::cout << "\n--- with hex format:\n\n";

    int value = 0xdeadbeef;

    std::cout << show(value, none) << std::endl;
    std::cout << show(hex(value), none) << std::endl;
    
    value = 01234;
    std::cout << show(oct(value), none) << std::endl;

    return 0;
}
