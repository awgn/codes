/* $Id: singleton-test.cc 466 2010-03-13 12:31:43Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <iostream>
#include <singleton.hpp>

struct s0: public more::singleton<s0, more::indestructible_singleton_type> 
{
    s0( base_type::tag )
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~s0()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

struct s1: public more::singleton<s1, volatile more::singleton_type> 
{
    int _M_value;

    s1( base_type::tag, int value = 0)
    :  _M_value(value)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~s1()
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

struct s2: public more::singleton<s2, const more::indestructible_singleton_type> {

    std::string _M_par1;
    std::string _M_par2;

    s2( base_type::tag, const std::string &p1 = std::string(), const std::string &p2 = std::string())
    : _M_par1(p1),
      _M_par2(p2)
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }

    ~s2() 
    {
        std::cout << __PRETTY_FUNCTION__ << std::endl;
    }
};

int 
main(int argc, char *argv[])
{
    s0 & r0 = s0::instance();
    volatile s1 & r1 = s1::instance(97);
    const s2 & r2 = s2::instance("hello","world");   // initialize the singleton

    std::cout << "\nsingleton instance (constructor on first-use):" << std::endl;

    std::cout << "r0 -> singleton instance @" << std::hex << &r0  << '\n';
    std::cout << "r1 -> singleton instance @" << std::hex << &r1  << '\n';
    std::cout << "r2 -> singleton instance @" << std::hex << &r2  << '\n';

    volatile s1 & q1 = s1::instance();
    const s2 & q2 = s2::instance();

    std::cout << "\nsingleton instances (access): " << std::endl;

    std::cout << "q1: slot @" << std::hex << &q1 << " value:" << std::dec << q1._M_value << '\n';
    std::cout << "q2: slot @" << std::hex << &q2 << " value:" << q2._M_par1 << ' ' << q2._M_par2 << '\n';

    std::cout << "\ntemplate singleton<T> instance:" << std::endl;

#ifdef ERR_0
    s1 a;  // <- instances of singleton are not allowed 
#endif
#ifdef ERR_1
    s1 b ( const_cast<const s1 &>(s1::instance()) ); // <- instances are not copyable
#endif
#ifdef ERR_2
    s1 & r5 = s1::instance(97); // cv-correctness
#endif
    return 0;
}
