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


int
main(int argc, char *argv[])
{
    std::cout << "[*] building the lazy_object<>..." << std::endl;
    more::lazy_object<derived,std::string> x("hello world");
   
    sleep(2);
     
    std::cout << "[*] building the real object..." << std::endl;
    x(); // build the object

    sleep(2);

    std::tr1::shared_ptr<derived> p = x.shared_from_this();
        
    std::cout << "[*] invoking methods on real object..." << std::endl;
    p->say();

    sleep(2);

    std::cout << "[*] destructing the object (by means of shared_ptr)..." << std::endl;
    return 0;
}
 
