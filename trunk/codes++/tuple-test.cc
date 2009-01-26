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
#include "tuple.hh"

using namespace mtp;

struct probe 
{
    probe() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    probe(int) { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    probe(const probe &) { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    probe &operator=(const probe &) { std::cout << __PRETTY_FUNCTION__ << std::endl; return *this; }
    ~probe() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

int main(int argc, char *argv[])
{
    std::cout << "[*] - tuple<TYPELIST(int)> - \n";
    tuple<TYPELIST(int)> test = make_tuple(1);
    std::cout <<  test.get<0>() << std::endl;    

    std::cout << "[*] - tuple<TYPELIST(int,int)> - \n";
    tuple<TYPELIST(int,int)> hello;    

    hello.get<0>() = 1;
    hello.get<1>() = 2;

    std::cout << hello.get<0>() << std::endl;
    std::cout << hello.get<1>() << std::endl;

    std::cout << "[*] - tuple<TYPELIST(int,int, std::string)> - \n";
    tuple<TYPELIST(int,int, std::string)> abc = make_tuple(1,2,std::string("3 (string)"));
    std::cout << abc.get<0>() << std::endl;
    std::cout << abc.get<1>() << std::endl;
    std::cout << abc.get<2>() << std::endl;

    std::cout << "[*] - tuple<TYPELIST(probe)> - \n";
    {
        tuple<TYPELIST(probe)> qwerty = make_tuple(probe(0));
    }
    std::cout << "[*] - done - \n";
    return 0;
}
