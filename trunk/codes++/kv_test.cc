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

TYPEMAP_KEY(unsigned_int);
TYPEMAP_KEY(integers);

TYPEMAP_KEY(booleans);
TYPEMAP_KEY(strings);

typedef TYPEMAP(unsigned_int, unsigned int,
                integers,     std::vector<int>, 
                booleans,     std::vector<bool>,
                strings,      std::vector<std::string>) SCRIPT;

struct script : public more::kv_parser<SCRIPT> {};

int
main(int argc, char *argv[])
{
    script abc;

    if ( !abc.parse("kv_test.txt", true /* strict */ ) ) {
        return -1;
    }

    std::cout << "-> " << unsigned_int::value() << " = " << abc.get<unsigned_int>() << std::endl;

    for(unsigned int i=0; i < abc.get<integers>().size(); i++) {
        std::cout << "-> " << integers::value() << " = " << abc.get<integers>()[i] << std::endl;
    }
    for(unsigned int i=0; i < abc.get<booleans>().size(); i++) {
        std::cout << "-> " << booleans::value() << " = " << abc.get<booleans>()[i] << std::endl;
    }
    for(unsigned int i=0; i < abc.get<strings>().size(); i++) {
        std::cout << "-> " << strings::value() << " = '" << abc.get<strings>()[i] << "'" << std::endl;
    }

    return 0;
}

