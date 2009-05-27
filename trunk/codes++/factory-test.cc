/* $Id: factory.hh 6 2009-01-26 10:31:15Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <factory.hh>

struct base 
{
    virtual ~base() {}
    virtual void hello_world()=0;
};

////////////////  default constructible classes  //////////////// 

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

////////////////  global factories: simple and 1-parameter  //////////////// 

more::factory<std::string, base> factory_0;                 // simple factory
more::factory<std::string, base, std::string> factory_1;    // factory of objects whose constructors accept a std::string 

namespace 
{
    // elegant registration: by means of the factory_register object

    more::factory_register<base, der1> _void_1(factory_0,"der1");
    more::factory_register<base, der2> _void_2(factory_0,"der2");
    more::factory_register<base, der3> _void_3(factory_1,"der3", std::string());
    more::factory_register<base, der4> _void_4(factory_1,"der4", std::string());
}

int
main(int argc, char *argv[])
{    
    // basic registration: by invoking the regist() method on the factory object, providing the "id" and the factory_allocator

    // factory_0.regist("der1", new more::factory_allocator<base, der1> );
    // factory_0.regist("der2", new more::factory_allocator<base, der2> );
    // factory_1.regist("der3", new more::factory_allocator<base, der3, std::string> );
    // factory_1.regist("der4", new more::factory_allocator<base, der4, std::string> );

    std::cout << "--- start here ---" << std::endl;

    {
        std::tr1::shared_ptr<base> p1 = factory_0("der1");
        std::tr1::shared_ptr<base> p2 = factory_0("der2");
        std::tr1::shared_ptr<base> p3 = factory_1("der3",std::string("hello"));
        std::tr1::shared_ptr<base> p4 = factory_1("der4",std::string("world"));

        p1->hello_world();
        p2->hello_world();
        p3->hello_world();
        p4->hello_world();
    }

    std::cout << "--- out of scope ---" << std::endl;

    factory_0.unregist("der2");
    factory_1.unregist("der4");
    
    std::cout << std::boolalpha << "der1 is_registered: " <<  factory_0.is_registered("der1") << std::endl;
    std::cout << std::boolalpha << "der2 is_registered: " <<  factory_0.is_registered("der2") << std::endl;
    std::cout << std::boolalpha << "der3 is_registered: " <<  factory_1.is_registered("der3") << std::endl;
    std::cout << std::boolalpha << "der4 is_registered: " <<  factory_1.is_registered("der4") << std::endl;

    return 0;
}
 
