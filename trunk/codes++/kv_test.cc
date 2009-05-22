/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <cstdlib>
#include <kv_parser.hh>

TYPEMAP_KEY_DEFAULT(unsigned_int, 3);   // set the default value for the key "unsigned_int" 

TYPEMAP_KEY(integers);

TYPEMAP_KEY(booleans);
TYPEMAP_KEY(strings);

TYPEMAP_KEY(block);
TYPEMAP_KEY(blocks);

    // <--- block --->
    namespace b
    {
        TYPEMAP_KEY(one);
        TYPEMAP_KEY(two);
        TYPEMAP_KEY(three);

        typedef TYPEMAP(one,    int,
                        two,    int, 
                        three,  int) BLOCK; 

        typedef more::kv::block<BLOCK, true /* strict: unknown key are parse errors */ > type;
    }

    // <--- blocks[] --->
    namespace bs
    {
        TYPEMAP_KEY(one);
        TYPEMAP_KEY(two);
        TYPEMAP_KEY(three);

        typedef TYPEMAP(one,    int,
                        two,    int, 
                        three,  int) BLOCK; 

        typedef more::kv::block<BLOCK, true /* strict: unknown key are parse errors */ > type;
    }


typedef TYPEMAP(unsigned_int, unsigned int,
                integers,     std::vector<int>, 
                booleans,     std::vector<bool>,
                strings,      std::vector<std::string>,
                block,        b::type,
                blocks,       std::vector<bs::type>) SCRIPT;

struct script : public more::kv::parser<SCRIPT, false /* non-strict: unknown key are ignored */ > {};

int
main(int argc, char *argv[])
{
    script abc;

    if ( !abc.parse("kv_test.txt") ) {
        return -1;
    }

    std::cout << "-> " << unsigned_int::value() << " = " << abc.get<unsigned_int>() << std::endl;
    std::cout << '\n';

    for(unsigned int i=0; i < abc.get<integers>().size(); i++) {
        std::cout << "-> " << integers::value() << " = " << abc.get<integers>()[i] << std::endl;
    }
    std::cout << '\n';
    for(unsigned int i=0; i < abc.get<booleans>().size(); i++) {
        std::cout << "-> " << booleans::value() << " = " << abc.get<booleans>()[i] << std::endl;
    }
    std::cout << '\n';
    for(unsigned int i=0; i < abc.get<strings>().size(); i++) {
        std::cout << "-> " << strings::value() << " = '" << abc.get<strings>()[i] << "'" << std::endl;
    }
    std::cout << '\n';

    std::cout << "   `-> " << block::value() << ":" << b::one::value()   << " = " << abc.get<block>().get<b::one>() << std::endl;
    std::cout << "   `-> " << block::value() << ":" << b::two::value()   << " = " << abc.get<block>().get<b::two>() << std::endl;
    std::cout << "   `-> " << block::value() << ":" << b::three::value() << " = " << abc.get<block>().get<b::three>() << std::endl;
    std::cout << std::endl;

    for(unsigned int i=0; i < abc.get<blocks>().size(); i++) {
        std::cout << "   `-> " << blocks::value() << ":" << bs::one::value()   << " = " << abc.get<blocks>()[i].get<bs::one>() << std::endl;
        std::cout << "   `-> " << blocks::value() << ":" << bs::two::value()   << " = " << abc.get<blocks>()[i].get<bs::two>() << std::endl;
        std::cout << "   `-> " << blocks::value() << ":" << bs::three::value() << " = " << abc.get<blocks>()[i].get<bs::three>() << std::endl;
        std::cout << std::endl;
    }

    return 0;
}

