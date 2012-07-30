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
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};
 
struct der1 : public base 
{
    der1() : base() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    ~der1()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }
    
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

struct der2 : public base 
{
    der2() : base() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    ~der2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }
 
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

////////////////  1 parameter-constructible classes (up to 5)  //////////////// 

struct der3 : public base 
{
    der3(const std::string &n) : base() 
    { std::cout << __PRETTY_FUNCTION__ << " -> " << n << std::endl; }

    ~der3() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

struct der4 : public base 
{
    der4(const std::string &m) : base() 
    { std::cout << __PRETTY_FUNCTION__ << " -> " << m << std::endl; }

    ~der4() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};


struct der5 : public base
{
    der5(std::string && m) : base() 
    { std::cout << __PRETTY_FUNCTION__ << " -> " << m << std::endl; }

    der5(const std::string & m) : base() 
    { std::cout << __PRETTY_FUNCTION__ << " -> " << m << std::endl; }
    
    ~der5() 
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};


////////////////  global factories: simple and 1-parameter  //////////////// 

more::factory<std::string, base> factory_0;                 // simple factory
more::factory<std::string, base, std::string> factory_1;    // factory of objects whose constructors accept a std::string 
more::factory<std::string, base, std::string &&> factory_2; // factory of objects whose constructors accept a r-value ref. to std::string 

namespace 
{
    // automatic hook: by means of the factory_register object

    more::factory_register<base, der1> _void_1_(factory_0,"der1");
    more::factory_register<base, der2> _void_2_(factory_0,"der2");
    more::factory_register<base, der3> _void_3_(factory_1,"der3");
    more::factory_register<base, der4> _void_4_(factory_1,"der4");
    more::factory_register<base, der5> _void_5_(factory_2,"der5");
}


Context(factory_test)
{
    Test(simple)
    {

    // basic hook: by invoking the regist() method on the factory object, providing the "id" and the factory_allocator

    std::cout << "--- start here ---" << std::endl;

    {
        auto p1 = factory_0("der1");
        std::unique_ptr<base> p2 = factory_0("der2");
        std::shared_ptr<base> p3 = factory_1.shared("der3",std::string("hello"));

        std::string hello("hello");

        auto p4 = factory_2("der5", std::move(hello));
        auto p5 = factory_2("der5", std::string("hello"));

        p1->hello_world();
        p2->hello_world();
        p3->hello_world();
        p4->hello_world();
        p5->hello_world();
    }

    std::cout << "--- out of scope ---" << std::endl;

    factory_0.unregist("der2");
    factory_1.unregist("der4");
    
    Assert(factory_0.is_registered("der1"), is_true());
    Assert(factory_0.is_registered("der2"), is_false());
    Assert(factory_1.is_registered("der3"), is_true());
    Assert(factory_1.is_registered("der4"), is_false());

    }
}
 
int
main(int argc, char *argv[])
{
    return yats::run(argc, argv);
}
 