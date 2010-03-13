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
#include <string>

#include <observer.hh>

using namespace more;

struct sub1 : public subject<> {};

struct sub2 : public subject< observer_opt::enable_shared_ptr > 
{
    sub2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 

    ~sub2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 
};

struct sub3 : public subject<int, const std::string &> 
{
    sub3()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 

    ~sub3()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 
};

//////////////////////////////////////////////////////

struct obs1 : public observer<>
{
    obs1(int n)
    : _M_value(n)
    {}

    virtual void
    update()
    {
        std::cout << __PRETTY_FUNCTION__  << " " << _M_value << std::endl;
    }

    int _M_value;
};

struct obs2 : public observer<>
{
    obs2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 

    ~obs2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 

    virtual void
    update()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

struct obs3 : public observer<int, const std::string &>
{
    obs3()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 

    ~obs3()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; } 

    virtual void
    update(int n, const std::string & h)
    {
        std::cout << __PRETTY_FUNCTION__ << ": param = " << n << ", '" << h << "'" << std::endl;
    }
};

int
main(int argc, char *argv[])
{
    std::cout << "\n[*] subject/observer: basic\n\n"; 
    sub1 general;

    obs1 soldier_1(1);
    obs1 soldier_2(2);
    obs1 soldier_3(3);

    std::cout << "    attach(3)" << std::endl;
    general.attach(&soldier_1);
    general.attach(&soldier_2);
    general.attach(&soldier_3);
    std::cout << "    notify()" << std::endl;
    general.notify();

    std::cout << "    detach() [3]" << std::endl;
    general.detach(&soldier_3);
    std::cout << "    notify()" << std::endl;
    general.notify(); 

    std::cout << "\n[*] subject/observer: subject with shared_ptr<>\n\n"; 

    {
        sub2 owner;
        {
            std::tr1::shared_ptr<observer<> > obs_1(new obs2);
            std::tr1::shared_ptr<observer<> > obs_2(new obs2);

            std::cout << "    attach(2)" << std::endl;
            owner.attach(obs_1);
            owner.attach(obs_2);

            std::cout << "    notify()" << std::endl;
            owner.notify();

            std::cout << "    detach() [delete the observer]" << std::endl;
            owner.detach(obs_1);
        }

        std::cout << "    destruct the subect [and all attached observers]" << std::endl;
    }
    
    std::cout << "\n[*] subject/observer: notify() with parameter(s)\n\n"; 
    {
        sub3 general;
        obs3 soldier;

        general.attach(&soldier);
        
        std::cout << "    notify(42,\"hello world\")" << std::endl;
        general.notify<int, const std::string &>(42, std::string("hello world") );
    }

    std::cout << "\n[*] done.\n";
    return 0;
}

