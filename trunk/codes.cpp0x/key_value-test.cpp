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

#include <key_value.hpp>

using namespace more::type;

MAP_KEY_VALUE(unsigned int, unsigned_int, 3);   // default value for the key  

MAP_KEY(std::vector<int>,               integers);
MAP_KEY(std::vector<bool>,              booleans);
MAP_KEY(std::list<std::string>,         strings);

typedef std::map<std::string,int> map_type;
MAP_KEY(map_type, associative);

typedef std::pair<int, int> pair_type;  
MAP_KEY(pair_type, pair_tuple);

    // <--- block --->
    namespace b
    {
        MAP_KEY(int, one);
        MAP_KEY(int, two);
        MAP_KEY(int, three);

        typedef more::kv::block<typemap<one::type, 
                                        two::type, 
                                        three::type>, 
                                        true /* strict: unknown key are parse errors */ > type;
    }

MAP_KEY(b::type, block);

    // <--- blocks[] --->
    namespace bs
    {
        MAP_KEY(int, one);
        MAP_KEY(int, two);
        MAP_KEY(int, three);

        typedef more::kv::block<typemap< one::type,
                                         two::type,
                                         three::type>,
                                         true /* strict: unknown key are parse errors */ > type;
    }

MAP_KEY(std::vector<bs::type>, blocks);

struct myparser : public more::kv::parser< typemap<unsigned_int::type,
                                                   integers::type,
                                                   booleans::type,
                                                   strings::type,
                                                   associative::type,
                                                   block::type,
                                                   blocks::type>,
                                                   false /* non-strict: unknown key are ignored */ > {};

int
main(int argc, char *argv[])
{
    myparser par;

    if ( !par.parse("key_value_test.txt") ) {
        return -1;
    }

    std::cout << "-> " << unsigned_int::value() << " = " << par.get<unsigned_int>() << std::endl;
    std::cout << std::endl;

    std::cout << "-> " << integers::value() << " = ";
    std::copy(par.get<integers>().begin(), par.get<integers>().end(), std::ostream_iterator<int>(std::cout," "));
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "-> " << booleans::value() << " = ";
    std::copy(par.get<booleans>().begin(), par.get<booleans>().end(), std::ostream_iterator<int>(std::cout," "));
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "-> " << strings::value() << " = ";
    std::copy(par.get<strings>().begin(), par.get<strings>().end(), std::ostream_iterator<std::string>(std::cout," - "));
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "   `-> " << block::value() << ":" << b::one::value()   << " = " << par.get<block>().get<b::one>() << std::endl;
    std::cout << "   `-> " << block::value() << ":" << b::two::value()   << " = " << par.get<block>().get<b::two>() << std::endl;
    std::cout << "   `-> " << block::value() << ":" << b::three::value() << " = " << par.get<block>().get<b::three>() << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    for(unsigned int i=0; i < par.get<blocks>().size(); i++) {
        std::cout << "   `-> " << blocks::value() << ":" << bs::one::value()   << " = " << par.get<blocks>()[i].get<bs::one>() << std::endl;
        std::cout << "   `-> " << blocks::value() << ":" << bs::two::value()   << " = " << par.get<blocks>()[i].get<bs::two>() << std::endl;
        std::cout << "   `-> " << blocks::value() << ":" << bs::three::value() << " = " << par.get<blocks>()[i].get<bs::three>() << std::endl;
        std::cout << std::endl;
    }

    std::cout << "-> " << associative::value() << " elem:" << par.get<associative>().size() << std::endl;

    auto it = par.get<associative>().begin();
    auto it_end = par.get<associative>().end();

    for(; it != it_end; ++it)
    {
        std::cout << "   key:" << (*it).first << " => value:" << (*it).second << std::endl; 
    }

    return 0;
}

