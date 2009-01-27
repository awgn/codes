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

struct der1 : public base 
{
    der1() : base() 
    {}
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

struct der2 : public base 
{
    der2() : base() 
    {}
    virtual void hello_world() { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};

int
main(int argc, char *argv[])
{
    more::factory<std::string, base> factory;

    factory.regist("der1", new more::factory_alloc<der1, base> );
    factory.regist("der2", new more::factory_alloc<der2, base> );

    std::tr1::shared_ptr<base> p1 = factory("der1");
    std::tr1::shared_ptr<base> p2 = factory("der2");

    if (!p1) {
        std::cout << "object not registered!" << std::endl;
    }
    
    p1->hello_world();
    p2->hello_world();

    factory.unregist("der2");

    std::cout << std::boolalpha << "     -> der1 registration: " <<  factory.is_registered("der1") << std::endl;
    std::cout << std::boolalpha << "     -> der2 registration: " <<  factory.is_registered("der2") << std::endl;

    return 0;
}
 
