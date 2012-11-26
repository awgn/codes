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
#include <factory.hpp>
#include <yats.hpp>
using namespace yats;

struct base 
{
    virtual ~base() {}
    virtual void hello_world()=0;
};

////////////////  default constructible classes  //////////////// 

struct der0  // error
{
    virtual ~der0() {}
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};
 
struct der1 : public base 
{
    der1() : base() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    virtual ~der1()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

struct der2 : public base 
{
    der2() : base() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    virtual ~der2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }
 
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

////////////////  1 parameter-constructible classes (up to 5)  //////////////// 

struct der3 : public base 
{
    der3(std::string n) 
    : base() 
    , value(std::move(n))
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    virtual ~der3() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    std::string value;

    virtual void hello_world() { std::cout << value << std::endl; }
};


////////////////  global factories: simple and 1-parameter  //////////////// 

more::factory<std::string, base> factory;                   // generic factory: for objects default-constructible or for types that accept a variadic number of arguments

namespace 
{
    // automatic hook: by means of the factory_register object

    more::factory_register<base, der1> _void_1_(factory,"der1");
    more::factory_register<base, der2> _void_2_(factory,"der2");
    more::factory_register<base, der3> _void_3_(factory,"der3", more::fac_args<std::string>());
}


Context(factory_test)
{
    Test(simple)
    {

    // basic hook: by invoking the regist() method on the factory object, providing the "id" and the factory_allocator

    std::cout << "--- start here ---" << std::endl;

    {
        auto p1 = factory("der1");
        auto p2 = factory("der2");
        auto p3 = factory("der3", std::string("ciao mondo!"));

        p1->hello_world();
        p2->hello_world();
        p3->hello_world();
    }

    std::cout << "--- out of scope ---" << std::endl;

    factory.unregist("der2");
    
    Assert(factory.is_registered("der1"), is_true());
    Assert(factory.is_registered("der2"), is_false());
    Assert(factory.is_registered("der3"), is_true());

    }
}
 
int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 
