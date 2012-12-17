/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

#include <iterator.hpp>

int
main(int, char *[])
{
    typedef std::map<int, std::string> map_type;

    map_type m { {1, "hello"}, {2, "world"}, {3, "abc"}};

    std::for_each(more::first_iterator(m.begin()),
                  more::first_iterator(m.end()),
                  [](int n) {

                        std::cout << n << std::endl;
                  });
    
    std::for_each(more::second_iterator(m.begin()),
                  more::second_iterator(m.end()),
                  [](std::string & n) {

                        std::cout << n << std::endl;
                  });

    std::vector< std::tuple<int, std::string, bool> > v;

    v.emplace_back(1, "hello", true);
    v.emplace_back(2, "world", false);
    
    auto it = more::tuple_element_iterator<2>(v.begin());
    
    std::cout << std::boolalpha;
    std::cout << it[0] << std::endl;
    std::cout << it[1] << std::endl;

    std::vector<const int *> vec(2);

    std::copy(more::address_iterator(more::first_iterator(m.begin())),
              more::address_iterator(more::first_iterator(m.end())),
              vec.begin());

    for(auto p : vec)
    {
        std::cout << *p << std::endl;
    }

    std::vector<std::string *> vec2(3);

    std::partial_sort_copy(more::address_iterator(more::second_iterator(m.begin())),
                           more::address_iterator(more::second_iterator(m.end())),
                           std::begin(vec2),
                           std::end(vec2), [](std::string *lhs, std::string *rhs) {
                                return *lhs < *rhs;
                           });

    for(auto p : vec2)
    {
        std::cout << *p << std::endl;
    }

    return 0;
}
 
