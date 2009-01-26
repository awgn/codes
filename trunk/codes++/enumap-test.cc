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
#include <enumap.hh>

struct table : more::enumap<table>
{
    enumap_init_(table,2);
    enumap_entry(hello, 0);
    enumap_entry(world, 1);
};


int
main(int argc, char *argv[])
{
    std::cout << "\nruntime:\n";
    std::cout << "    hello -> " << table::eval("hello") << std::endl;
    std::cout << "    world -> " << table::eval("world") << std::endl;

    std::cout << "    0 <- " << table::eval(0) << std::endl;
    std::cout << "    1 <- " << table::eval(1) << std::endl;

    std::cout << "\ncompile-time:\n";
    std::cout << "    table::hello => " << table::hello << std::endl;
    std::cout << "    table::world => " << table::world << std::endl;

    std::cout << "    0 <= " << table::get<0>() << std::endl;
    std::cout << "    1 <= " << table::get<1>() << std::endl;

    std::cout << "\ndirect map:\n";
    table::direct_map::iterator it = table::direct.begin();
    for(; it != table::direct.end(); ++it)
    {
        std::cout << "   <" << it->first << "," << it->second << ">\n";
    }

    std::cout << "\nreverse map:\n";
    table::reverse_map::iterator ot = table::reverse.begin();
    for(; ot != table::reverse.end(); ++ot)
    {
        std::cout << "   <" << ot->first << "," << ot->second << ">\n";
    }

    std::cout << "\ntable::has(\"hello\") = " << std::boolalpha << table::has("hello") << std::endl;
    std::cout << "table::has(0) = " << std::boolalpha << table::has(0) << std::endl;

    return 0;
}
 
