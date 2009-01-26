/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <function-factory.hh>

struct test1
{
    int state;
    test1()
    : state(0)
    {}    

    int operator()(int i)
    {
        state++;
        if (i)
            return i;
        else
            return state;
    }

};

struct test2
{
    int operator()(int)
    {
        return 4;
    }
};


generic::functionFactory<std::string, int(int)> the_factory;
namespace {
    generic::functionFactory<std::string, int(int)>::register_factory _one_(the_factory, "one", test1() );
    generic::functionFactory<std::string, int(int)>::register_factory _two_(the_factory, "two", test2() );
}

int main(int argc, char *argv[])
{
    std::tr1::function<int(int)> one = the_factory("one");
    std::tr1::function<int(int)> two = the_factory("two");

    std::cout << one(1) << std::endl;
    std::cout << one(2) << std::endl;
    std::cout << one(0) << std::endl;

    std::cout << two(0) << std::endl;

    return 0;
}

