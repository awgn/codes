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

#include "generic_type.hpp"

GENERIC_TYPE ( Test, 
                     (Simple, int),
                     (Pair,   int, std::string)
             )

int
main(int argc, char *argv[])
{
    Test x;

    std::cin  >> x;
    std::cout << x << std::endl;

    switch(x.type())
    {
    case Test::Simple:
        {
            auto &x = x.arg_as<int>();
            std::cout << "Simple argument: " << std::get<0>(x) << std::endl;
        } break;
    case Test::Pair:
        {
            auto &xs = x.arg_as<int, std::string>();
            std::cout << "Pair arguments: " << std::get<0>(xs) << ", " << std::get<1>(xs) << std::endl;
        } break;
    case Test::unknown:
        throw std::runtime_error("Unknown");
        break;
    }

    return 0;
}

