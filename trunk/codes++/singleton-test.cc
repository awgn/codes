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
#include <singleton.hh>

struct s0: public more::singleton<s0, more::singleton_type> 
{
    s0( singleton_tag tag )
    : more::singleton<s0, more::singleton_type>(tag)
    {}
};

struct s1: public more::singleton<s1, volatile more::singleton_type, TYPELIST(int)> 
{
    int _M_value;

    s1( singleton_tag tag, int value)
    : more::singleton<s1, volatile more::singleton_type, TYPELIST(int) >(tag),
      _M_value(value)
    {}
};

struct s2: public more::singleton<s2, const more::singleton_type, TYPELIST(std::string, std::string) > {

    std::string _M_par1;
    std::string _M_par2;

    s2( singleton_tag tag, const std::string &p1, const std::string &p2)
    : more::singleton<s2, const more::singleton_type, TYPELIST(std::string, std::string) > (tag),
      _M_par1(p1),
      _M_par2(p2)
      {}
};


template <typename T>
struct s3: public more::singleton< s3<T>, more::singleton_type, TYPELIST(T) > 
{
    T _M_value;

    s3( typename more::singleton< s3<T>, more::singleton_type, TYPELIST(T) >::singleton_tag tag, T value )
    : more::singleton< s3<T>, more::singleton_type, TYPELIST(T)>(tag),
      _M_value(value)
    {}
};




int 
main(int argc, char *argv[])
{

#define RED     "\E[0;31;1m"
#define RESET   "\E[0m"

    s0 & r0 = s0::instance();
    volatile s1 & r1 = s1::instance(97);
    const s2 & r2 = s2::instance("hello","world");   // initialize the singleton

    std::cout << RED << "\nsingleton instance (constructor on first-use):" << RESET << std::endl;

    std::cout << "r0 -> singleton instance @" << std::hex << &r0  << '\n';
    std::cout << "r1 -> singleton instance @" << std::hex << &r1  << '\n';
    std::cout << "r2 -> singleton instance @" << std::hex << &r2  << '\n';

    volatile s1 & q1 = s1::instance();
    const s2 & q2 = s2::instance();

    std::cout << RED << "\nsingleton instances (access): " << RESET << std::endl;

    std::cout << "q1: slot @" << std::hex << &q1 << " value:" << std::dec << q1._M_value << '\n';
    std::cout << "q2: slot @" << std::hex << &q2 << " value:" << q2._M_par1 << ' ' << q2._M_par2 << '\n';

    std::cout << RED << "\ntemplate singleton<T> instance:" << RESET << std::endl;

    s3<double> & t3 = s3<double>::instance(11.2);

    std::cout << "t3: slot @" << std::hex << &t3 << " value:" << std::dec << t3._M_value << '\n';

#ifdef ERR_0
    s1 a;  // <- instances of singleton are not allowed 
#endif
#ifdef ERR_1
    s1 b ( const_cast<const s1 &>(s1::instance()) ); // <- instances are not copyable
#endif
#ifdef ERR_2
    s1 & r1 = s1::instance(97); // cv-correctness
#endif
    return 0;
}
