/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <persistent.hpp>

#include <iostream>
#include <stdexcept>

#include <map>

std::map<std::string, int> maptype = {
    { "uint8_t",    1 },
    { "int8_t",     2 },
    { "uint16_t",   3 },
    { "int16_t",    4 },
    { "uint32_t",   5 },
    { "int32_t",    6 },
    { "uint64_t",   7 },
    { "int64_t",    8 }
    };

int
main(int argc, char *argv[])
try
{
    if (argc < 2)
        throw std::runtime_error((std::string("usage: ") + argv[0]) + " -r TYPE PATH");

    if (argv[1] == std::string("-h"))
    {
        throw std::runtime_error((std::string("usage: ") + argv[0]) + " -r TYPE PATH");
    }

    if (argv[1] == std::string("-r"))
    {
        if (argc < 4)
            throw std::runtime_error(std::string("usage: -r TYPE PATH (missing arguments)"));

        auto it = maptype.find(argv[2]);
        if (it == std::end(maptype))
            throw std::runtime_error(std::string(argv[2]) + ": unknown type");
         
        switch(it->second)
        {
            case 1: {
                more::persistent<uint8_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;
            } break;
            case 2: {
                more::persistent<int8_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;
            } break;
            case 3: {
                more::persistent<uint16_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;
            } break;
            case 4: {
                more::persistent<int16_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;

            } break;
            case 5: {
                more::persistent<uint32_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;

            } break;
            case 6: {
                more::persistent<int32_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;

            } break;
            case 7: {
                more::persistent<uint64_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;

            } break;
            case 8: {
                more::persistent<int64_t> value(argv[3]);
                std::cout << argv[2] << "[" << argv[3] << "] = " << value.get() << std::endl;
            } break;

        }

        exit (0);
    }

    throw std::runtime_error(std::string(argv[1]) + ": unknown option");

    return 0;
}
catch(std::exception &e)
{
    std::cerr << e.what() << std::endl;
}
