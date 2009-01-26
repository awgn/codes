/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

// an ordinary foolish code... TH 1:02 7.26 2007 :/
//

#include <iostream>

enum type {
    zero,
    one,
    two
};

extern "C" void fun0() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
           void fun1() { std::cout << __PRETTY_FUNCTION__ << std::endl; }

struct test {
    template <type T> static void fun(); 
    static void fun2() __attribute__((used)) { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

// compile time dispatcher...
//
template<> void test::fun<zero>() __attribute__((alias("fun0"))); 
template<> void test::fun<one>()  __attribute__((alias("_Z4fun1v"))); 
template<> void test::fun<two>()  __attribute__((alias("_ZN4test4fun2Ev"))); 

int main()
{
    test::fun<zero>();
    test::fun<one>();
    test::fun<two>();
}
