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
#include <vector>
#include <algorithm>

#include <lazy_object.hh>

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

    virtual void say() = 0;
};

struct derived : public base
{

    derived(const std::string &h)
    : _M_msg(h)
    {
        std::cout << __PRETTY_FUNCTION__ << ": " <<  h << std::endl;
    }

    ~derived()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    virtual void say()
    {
        std::cout << _M_msg << "!" << std::endl;
    }

    std::string _M_msg;
};


typedef more::lazy_object<derived,std::string> lazy_derived;

#include <tr1/functional>

int
main(int argc, char *argv[])
{

    std::vector<lazy_derived> vec;

    std::cout << "[*] building the lazy_object<>..." << std::endl;

    more::lazy_object<derived,std::string> x("hello");
    more::lazy_object<derived,std::string> y("world");
  
    vec.push_back(x);
    vec.push_back(y);

    sleep(2);
     
    std::cout << "[*] building real objects..." << std::endl;
    std::for_each(vec.begin(), vec.end(), std::tr1::mem_fn(&more::lazy_object<derived,std::string>::operator()) );

    sleep(2);

    std::tr1::shared_ptr<derived> p = vec[0].shared_from_this();
    std::tr1::shared_ptr<derived> q = vec[1].shared_from_this();
        
    std::cout << "[*] invoking methods on real object..." << std::endl;
    p->say();
    q->say();

    sleep(2);

    std::cout << "[*] destructing the object (by means of shared_ptr)..." << std::endl;
    return 0;
}
 
