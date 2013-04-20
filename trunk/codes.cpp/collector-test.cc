/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <collector.hh>

#include <string>
#include <iostream>

struct test : public more::collector<std::string, test>
{
    test(const std::string &x)
    : more::collector<std::string, test>(x),
      id(x)
    {
        std::cout << __PRETTY_FUNCTION__ << ": " << id << std::endl;
    }

    ~test()
    {
        std::cout << __PRETTY_FUNCTION__ << ": " << id << std::endl;
    }

    const std::string id;
};

test obj_0("bss");

int main(int, char *[])
{
    std::cout << "main: prolog ------------------ \n";

    test obj_1("auto");

    test("temp"); 

    new test("heap1");
    new test("heap2");

    test * x = new test("heap3");
    delete x;

    test * y = more::collector<std::string, test>::get("heap1");
    delete y;

    std::cout << "main: epilog ------------------ \n";
    return 0;
}
