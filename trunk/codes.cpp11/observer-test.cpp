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

#include <observer.hpp>

using namespace more;

struct sub1 : public subject<> {};

struct sub2 : public subject<>
{
    sub2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    ~sub2()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }
};


struct sub3 : public subject<int, std::string>
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
    : m_value(n)
    {}

    virtual void
    update()
    {
        std::cout << __PRETTY_FUNCTION__  << " " << m_value << std::endl;
    }

    int m_value;
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

struct obs3 : public observer<int, std::string>
{
    obs3()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    ~obs3()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    virtual void
    update(int n, std::string h)
    {
        std::cout << __PRETTY_FUNCTION__ << ": param = " << n << ", '" << h << "'" << std::endl;
    }
};

int
main(int, char *[])
{
     std::cout << "\n[*] subject/observer: basic\n\n";
     sub1 captain;

     obs1 soldier_1(1);
     obs1 soldier_2(2);
     obs1 soldier_3(3);

     std::cout << "    attach(3)" << std::endl;
     captain.attach(&soldier_1);
     captain.attach(&soldier_2);
     captain.attach(&soldier_3);

     std::cout << "    notify()" << std::endl;
     captain.notify();

     std::cout << "    detach() [3]" << std::endl;
     captain.detach(&soldier_3);
     std::cout << "    notify()" << std::endl;
     captain.notify();

     std::cout << "\n[*] subject/observer: subject with shared_ptr<>\n\n";

     {
         sub2 owner;
         {
             std::shared_ptr<observer<> > obs_1(new obs2);
             std::shared_ptr<observer<> > obs_2(new obs2);

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
         sub3 captain;
         obs3 soldier;

         captain.attach(&soldier);

         std::cout << "    notify(42,\"hello world\")" << std::endl;
         captain.notify(42, std::string("hello world"));
     }

     std::cout << "\n[*] done.\n";
     return 0;
}

