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

GENERIC_TYPE ( Test, (Single),
                     (Simple, int),
                     (Pair,   int, std::string)
             )

int
main(int argc, char *argv[])
{
    Test x[2];

    std::cin  >> x[0] >> x[1];
    std::cout << x[0] << ' ' << x[1] << std::endl;

    for(auto i = 0; i < 2; ++i)
    {
        switch(x[i].type())
        {
        case Test::Single:
            {
                std::cout << "-> Single"  << std::endl;

            } break;
        case Test::Simple:
            {
                auto & xs = x[i].data_as<int>();
                std::cout << "-> Simple " << std::get<0>(xs) << std::endl;

            } break;
        case Test::Pair:
            {
                auto &xs = x[i].data_as<int, std::string>();
                std::cout << "-> Pair " << std::get<0>(xs) << ", " << std::get<1>(xs) << std::endl;

            } break;
        case Test::unknown:
            {
                throw std::runtime_error("Unknown");
            } break;
        }
    }

    Test const &y = x[0];
    if (y.type() == Test::Simple)
    {
        auto & xs = y.data_as<int>();
        std::cout << "-> const Simple " << std::get<0>(xs) << std::endl;
    }

    return 0;
}

