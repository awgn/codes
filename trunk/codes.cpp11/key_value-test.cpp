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
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <vector>
#include <list>
#include <map>
#include <set>

#include <key_value.hpp>

MAP_KEY_VALUE(unsigned int, unsigned_int, 3);   // default value for the key  

MAP_KEY(std::vector<int>, integers);

MAP_KEY(std::vector<bool>, booleans);
MAP_KEY(std::list<std::string>, strings);

MAP_KEY(std::set<int>, intset);

typedef std::map<std::string,int> map_type;
MAP_KEY(map_type, associative);
 
typedef std::pair<double,double> pair_type;  
MAP_KEY(pair_type, simple_pair);

typedef std::tuple<bool, int, double, std::string> tuple_type;
MAP_KEY(tuple_type, simple_tuple);

namespace simple_block {

    MAP_KEY(int, first);
    MAP_KEY(int, second);

    typedef more::key_value_pack<first, second>  type;
}

MAP_KEY(simple_block::type, block); 

// smart pointer support

MAP_KEY(double *, raw_double);
MAP_KEY(std::shared_ptr<int>, shared_int);

MAP_KEY(const char *, literal);


MAP_KEY(std::vector<std::vector<int>>, matrix);

typedef more::key_value_pack<unsigned_int,
                               integers,
                               booleans,
                               strings, 
                               intset,
                               associative,
                               simple_pair,
                               simple_tuple,
                               block,
                               raw_double,
                               shared_int,
                               literal,
                               matrix> my_config;

int
main(int, char *[])
{
    const my_config conf("key_value_test.txt", more::key_value_opt::non_strict());

    // or...
    // if (!conf.open("key_value_test.txt") ) {
    //     return -1;
    // }

    std::cout << "-> " << unsigned_int::str() << " = " << more::get<unsigned_int>(conf) << std::endl;

    std::cout << "-> " << integers::str() << " = ";
    std::copy(conf.get<integers>().begin(), conf.get<integers>().end(), std::ostream_iterator<int>(std::cout," "));
    std::cout << std::endl;

    std::cout << "-> " << booleans::str() << " = ";
    std::copy(conf.get<booleans>().begin(), conf.get<booleans>().end(), std::ostream_iterator<int>(std::cout," "));
    std::cout << std::endl;

    std::cout << "-> " << strings::str() << " = ";
    std::copy(conf.get<strings>().begin(), conf.get<strings>().end(), std::ostream_iterator<std::string>(std::cout,"|"));
    std::cout << std::endl;

    std::cout << "-> " << associative::str() << " elem:" << conf.get<associative>().size() << std::endl;
    auto it = conf.get<associative>().begin();
    auto it_end = conf.get<associative>().end();
    for(; it != it_end; ++it)
    {
        std::cout << "   key:" << (*it).first << " -> value:" << (*it).second << std::endl; 
    }

    std::cout << "-> " << intset::str() << " elem:" << conf.get<intset>().size() << std::endl;
    auto sit = conf.get<intset>().begin();
    auto sit_end = conf.get<intset>().end();
    for(; sit != sit_end; ++sit)
    {
        std::cout << "   value:" << (*sit) << std::endl; 
    }

    std::cout << "-> " << simple_pair::str() << " = (" << conf.get<simple_pair>().first << " , " << conf.get<simple_pair>().second << ")" << std::endl;

    std::cout << "-> " << simple_tuple::str() << " = (" << std::get<0>( more::get<simple_tuple>(conf) ) << " , " <<
    std::get<1>( more::get<simple_tuple>(conf) ) << " , " <<
    std::get<2>( more::get<simple_tuple>(conf) ) << " , " <<
    std::get<3>( more::get<simple_tuple>(conf) ) << ")" << std::endl;

    std::cout << "   block -> first:  " << more::get<simple_block::first>(more::get<block>(conf)) << '\n' <<
    "            second: " << more::get<simple_block::second>(more::get<block>(conf)) << std::endl;

    std::cout << " -> " << raw_double::str() << " => " << std::boolalpha << static_cast<bool>(more::get<raw_double>(conf));
    if (more::get<raw_double>(conf))
        std::cout << " value:" << *more::get<raw_double>(conf);
    std::cout << std::endl;

    std::cout << " -> " << shared_int::str() << " => " << std::boolalpha << static_cast<bool>(more::get<shared_int>(conf));
    if( more::get<shared_int>(conf) )
        std::cout << " value:" << *more::get<shared_int>(conf);
    std::cout << std::endl;

    std::cout << " -> " << literal::str() << " => " << std::boolalpha << static_cast<bool>(more::get<literal>(conf));
    if( more::get<literal>(conf) )
        std::cout << " value:" << more::get<literal>(conf);
    std::cout << std::endl;

    std::cout << " -> " << matrix::str() << " =\n";
    auto m = more::get<matrix>(conf);
    std::cout << "     " << m[0][0] << ' ' << m[0][1] << std::endl;
    std::cout << "     " << m[1][0] << ' ' << m[1][1] << std::endl;

    return 0;
}

