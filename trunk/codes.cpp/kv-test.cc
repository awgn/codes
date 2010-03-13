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

#include <kv_parser.hh>

TYPEMAP_KEY(unsigned int, unsigned_int, 3);   // default value for the key  

TYPEMAP_KEY(std::vector<int>,         integers);
TYPEMAP_KEY(std::vector<bool>,        booleans);
TYPEMAP_KEY(std::list<std::string>,   strings);

typedef std::map<std::string,int> map_type;
TYPEMAP_KEY(map_type, associative);

typedef std::pair<int, int> pair_type;
TYPEMAP_KEY(pair_type, pair_tuple);

    // <--- block --->
    namespace b
    {
        TYPEMAP_KEY(int, one);
        TYPEMAP_KEY(int, two);
        TYPEMAP_KEY(int, three);

        typedef TYPEMAP_KEY_LIST(one, two, three) BLOCK; 

        typedef more::kv::block<BLOCK, true /* strict: unknown key are parse errors */ > type;
    }

TYPEMAP_KEY(b::type, block);

    // <--- blocks[] --->
    namespace bs
    {
        TYPEMAP_KEY(int, one);
        TYPEMAP_KEY(int, two);
        TYPEMAP_KEY(int, three);

        typedef TYPEMAP_KEY_LIST(one, two, three) BLOCK; 

        typedef more::kv::block<BLOCK, true /* strict: unknown key are parse errors */ > type;
    }

TYPEMAP_KEY(std::vector<bs::type>, blocks);

typedef TYPEMAP_KEY_LIST(unsigned_int, integers, booleans, strings, block, blocks, associative) SCRIPT;

struct script : public more::kv::parser<SCRIPT, false /* non-strict: unknown key are ignored */ > {};

int
main(int argc, char *argv[])
{
    script par;

    if ( !par.parse("kv_test.txt") ) {
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

    associative::value_type::const_iterator it = par.get<associative>().begin();
    associative::value_type::const_iterator it_end = par.get<associative>().end();

    for(; it != it_end; ++it)
    {
        std::cout << "   key:" << (*it).first << " => value:" << (*it).second << std::endl; 
    }

    return 0;
}

