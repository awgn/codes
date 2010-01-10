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

#include <kv_parser.hh>

TYPEMAP_KEY(unsigned int, unsigned_int, 3);   // default value for the key  

TYPEMAP_KEY(std::vector<int>,         integers);
TYPEMAP_KEY(std::vector<bool>,        booleans);
TYPEMAP_KEY(std::list<std::string>,   strings);


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

typedef TYPEMAP_KEY_LIST(unsigned_int, integers, booleans, strings, block, blocks) SCRIPT;

struct script : public more::kv::parser<SCRIPT, false /* non-strict: unknown key are ignored */ > {};

int
main(int argc, char *argv[])
{
    script abc;

    if ( !abc.parse("kv_test.txt") ) {
        return -1;
    }

    std::cout << "-> " << unsigned_int::value() << " = " << abc.get<unsigned_int>() << std::endl;
    std::cout << std::endl;

    std::cout << "-> " << integers::value() << " = ";
    std::copy(abc.get<integers>().begin(), abc.get<integers>().end(), std::ostream_iterator<int>(std::cout," "));
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "-> " << booleans::value() << " = ";
    std::copy(abc.get<booleans>().begin(), abc.get<booleans>().end(), std::ostream_iterator<int>(std::cout," "));
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "-> " << strings::value() << " = ";
    std::copy(abc.get<strings>().begin(), abc.get<strings>().end(), std::ostream_iterator<std::string>(std::cout," - "));
    std::cout << std::endl;
    std::cout << std::endl;

    std::cout << "   `-> " << block::value() << ":" << b::one::value()   << " = " << abc.get<block>().get<b::one>() << std::endl;
    std::cout << "   `-> " << block::value() << ":" << b::two::value()   << " = " << abc.get<block>().get<b::two>() << std::endl;
    std::cout << "   `-> " << block::value() << ":" << b::three::value() << " = " << abc.get<block>().get<b::three>() << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;

    for(unsigned int i=0; i < abc.get<blocks>().size(); i++) {
        std::cout << "   `-> " << blocks::value() << ":" << bs::one::value()   << " = " << abc.get<blocks>()[i].get<bs::one>() << std::endl;
        std::cout << "   `-> " << blocks::value() << ":" << bs::two::value()   << " = " << abc.get<blocks>()[i].get<bs::two>() << std::endl;
        std::cout << "   `-> " << blocks::value() << ":" << bs::three::value() << " = " << abc.get<blocks>()[i].get<bs::three>() << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

