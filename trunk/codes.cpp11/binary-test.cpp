/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include "binary.hpp"

#include <iostream>
#include <memory>

namespace my
{
    struct test
    {
        char c;
    };

    inline void encode(test n, void *out)
    {
        auto p = reinterpret_cast<test *>(out);
           * p = n;
    }
    inline void decode(const void *in, test &n)
    {
        auto p = reinterpret_cast<test const *>(in);
        n = *p;
    }
    size_t size_of(test)
    {
        return 1;
    }
}


int
main(int, char *[])
{
    char buffer[16];

    more::binary bin(buffer, 16);

    my::test c { 'c' };

    bin.put(42U);
    bin.put(std::string("hello"));
    bin.put("world");
    bin.put(c);

    bin.reset();

    std::cout << bin.get<uint32_t>() << std::endl;
    std::cout << bin.get<std::string>(5) << std::endl;

    std::unique_ptr<char> p{ bin.get<char *>(5) };
    std::cout << p.get() << std::endl;

    std::cout << bin.get<my::test>().c << std::endl;

    return 0;
}

