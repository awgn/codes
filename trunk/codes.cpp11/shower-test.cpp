/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include "shower.hpp"

struct test
{
    int a;
    char b;
    std::string c;
};


int
main(int argc, char *argv[])
{
    // more::shower<test, int test::*, char test::*, std::string test::*> show_test( std::make_pair(std::string("a"), &test::a),
    //                                                                               std::make_pair(std::string("b"), &test::b),
    //                                                                               std::make_pair(std::string("c"), &test::c));


    // auto show_test = more::make_shower<test>(std::make_pair(std::string("a"), &test::a),
    //                                          std::make_pair(std::string("b"), &test::b),
    //                                          std::make_pair(std::string("c"), &test::c));


    auto show_test = MAKE_SHOW(test, a, b, c);

    test x = { 42, 'x', "hello world" };

    std::cout << show_test(x) << std::endl;

    return 0;
}

