/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <unistd.h>

#include <iostream>
#include <vector>
#include <algorithm>

#include <tr1/functional>

#include <lazy_object.hh>

//////////////////// base <- derived ... 

struct base 
{
    base()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
    virtual ~base()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void say() const = 0;
};

struct derived : public base
{

    derived(const std::string &h)
    : m_msg(h)
    {
        std::cout << "`->" <<  __PRETTY_FUNCTION__ << std::endl;
    }

    ~derived()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void say() const
    {
        std::cout << m_msg << "!" << std::endl;
    }

    std::string m_msg;
};



typedef more::lazy_object<derived> lazy_derived;

void test_upcast(const more::lazy_object<base> & lo)
{
    std::tr1::shared_ptr<base> p = lo.shared_from_this();
    p->say();
}


int
main(int, char *[])
{

    std::vector<lazy_derived> vec;

    std::cout << "[*] building the lazy_object<>..." << std::endl;

    more::lazy_object<derived> x(std::string("hello"));
    more::lazy_object<derived> y(std::string("world"));

    vec.push_back(x);
    vec.push_back(y);

    sleep(2);
     
    std::cout << "[*] building real objects..." << std::endl;

    // to build the object call ctor<> function as:
    //
    // x.ctor() <- if the class has a default constructor
    //
    // x.ctor<std::string>(); <- invoke derived(const std::string &) constructor
    //

    std::for_each(vec.begin(), vec.end(), std::tr1::mem_fn(&more::lazy_object<derived>::ctor<std::string> /* ctor signature */ ) );

    sleep(2);

    std::cout << "[*] test upcast lazy_objects..." << std::endl;

    x.ctor<std::string>();
    y.ctor<std::string>();

    test_upcast(x);
    test_upcast(y);

    std::cout << "[*] invoking methods on real object..." << std::endl;

    std::tr1::shared_ptr<base> p = vec[0].shared_from_this();
    std::tr1::shared_ptr<base> q = vec[1].shared_from_this();
        
    p->say();
    q->say();

    sleep(2);

    std::cout << "[*] destructing the object (by means of shared_ptr)..." << std::endl;
    return 0;
}
 
