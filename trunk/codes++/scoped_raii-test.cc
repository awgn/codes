/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <scoped_raii.hh>
#include <iostream>
#include <string>

struct device 
{
    void hello(const std::string &name)
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "Hello " << name << "!" << std::endl;
    }

    void bye()
    {
        // std::cout << __PRETTY_FUNCTION__ << std::endl;
        std::cout << "Goodbye." << std::endl;
    }
};

template <typename T>
class scoped_greeting : public more::scoped_raii_1<T, const std::string &, &T::hello, &T::bye> 
{
public:
    scoped_greeting(T &dev, const std::string &whom)
    : scoped_greeting<T>::base_type(dev,whom)
    {}
};


int
main(int argc, char *argv[])
{
    device x;
    scoped_greeting<device> abc(x, "world");

    return 0;
}
 
