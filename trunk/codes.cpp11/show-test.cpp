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
show(const test &t)
{
    return "test " + show(t.value);    
}


int 
main(int, char *[])
{
    std::cout << "--- basic (only values):\n\n";

    uint8_t uc = '\x42';

    const uint8_t ua[3] = {1, 2, 3};

    std::cout << show(uc) << std::endl; 
    std::cout << show(ua) << std::endl; 

    std::cout << show("hello world") << std::endl; 
    std::cout << show(std::string("hello world")) << std::endl; 

    std::cout << show((void *)0xdeadbeef) << std::endl; 

    auto p = std::make_pair("hello world", 42);

    std::cout << show(p) << std::endl; 
    
    std::vector<int> v1;

    v1.push_back(0);
    v1.push_back(1);
    v1.push_back(2);

    std::cout << show(v1) << std::endl;

    std::vector<char> v2;

    v2.push_back('a');
    v2.push_back('b');
    v2.push_back('c');

    std::cout << show(v2) << std::endl;

    std::list<std::pair<int,int> > l1;

    l1.push_back( std::make_pair(0,0) );
    l1.push_back( std::make_pair(1,1) );
    l1.push_back( std::make_pair(2,2) );

    std::cout << show(l1) << std::endl;

    std::map<std::string,int> m;

    m["hello"]=0;
    m["world"]=1;

    std::cout << show(m) << std::endl;

    std::cout << show(std::tuple<int,double>(1,1.1)) << std::endl;
    std::cout << show(std::array<int,3>()) << std::endl;

    std::cout << show(std::chrono::system_clock::now()) << std::endl;
    std::cout << show(std::chrono::milliseconds(10)) << std::endl;
    std::cout << show(test{"abc"}) << std::endl;

    int c_array[3] = {1,2,3};
    
    std::cout << "\n--- with typename:\n\n";

    auto c = std::make_tuple(std::vector<int>{1,2}, std::make_pair(1,2), std::chrono::seconds(1), std::make_tuple(1,2));
    
    std::cout << show(c, "") << std::endl;
    std::cout << show(c_array, "") << std::endl;
    
    std::cout << "\n--- with custom name:\n\n";
    
    std::cout << show(c_array, "C_array") << std::endl;
    std::cout << show(42, "Integer:") << std::endl;
    

    return 0;
}
