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

struct singleton1: protected more::singleton<singleton1> 
{
    SINGLETON_CTOR(singleton1) {}
};


class singleton2 : protected more::singleton<singleton2> 
{
    int __par1;
    int __par2;

public:
    SINGLETON_CTOR(singleton2), __par1(0), __par2(1) {}

    const int par1() const 
    { return __par1; }

    const int par2() const 
    { return __par2; }
};

class singleton3 : protected more::singleton<singleton3, volatile more::singleton_type > {

    int __par1;
    int __par2;

public:
    VOLATILE_SINGLETON_CTOR(singleton3, int _par1, int _par2 = 0 ), __par1(_par1), __par2(_par2) {}

    const int par1() const volatile 
    { return __par1; }

    const int par2() const volatile 
    { return __par2; }
};


template <typename T>
class singleton4 : protected more::singleton<singleton4<T>, const more::singleton_type > {

    T __par1;
    T __par2;

public:
    CONST_TEMPLATE_SINGLETON_CTOR(singleton4, T _par1, T _par2 = T() ), __par1(_par1), __par2(_par2) {}

    const T par1() const 
    { return __par1; }

    const T par2() const 
    { return __par2; }
};


int 
main(int argc, char *argv[])
{

#define RED     "\E[0;31;1m"
#define RESET   "\E[0m"

    singleton1 &ref1 = singleton1::instance();
    singleton2 &ref2 = singleton2::instance();
    volatile singleton3 &ref3 = singleton3::instance<int,int>(0,1);           // singleton: slot signature <int,int> 

    std::cout << RED << "\nsingleton instance..." << RESET << std::endl;

    std::cout << "ref1 -> singleton instance @" << std::hex << &ref1  << '\n';
    std::cout << "ref2 -> singleton instance @" << std::hex << &ref2  << '\n';
    std::cout << "ref3 -> singleton instance @" << std::hex << &ref3  << '\n';

    volatile singleton3 &ref4 = singleton3::instance<int>(1);               // singleton: slot <int> 

    std::cout << RED << "\nsingleton instances: slot <int,int>,  slot <int>" << RESET << std::endl;

    std::cout << "ref3: slot @" << std::hex << &ref3 << " par:" << std::dec << ref3.par1() << ',' << ref3.par2() << '\n';

    std::cout << RED << "\ncv-qualified singleton instances: " << RESET << std::endl;

    std::cout << "ref4: slot @" << std::hex << &ref4 << " par:" << std::dec << ref4.par1() << ',' << ref4.par2() << '\n';

    volatile singleton3 & ref5 = singleton3::instance<int,int>();
    volatile singleton3 & ref6 = singleton3::instance<int>();

    std::cout << "ref5: slot @" << std::hex << &ref5 << " par:" << std::dec << ref5.par1() << ',' << ref5.par2() << '\n';
    std::cout << "ref6: slot @" << std::hex << &ref6 << " par:" << std::dec << ref6.par1() << ',' << ref6.par2() << '\n';

    std::cout << RED << "\ntemplate singleton<T> instance:" << RESET << std::endl;

    const singleton4<double> & ref7 = singleton4<double>::instance<double,double>(1.1,1.2);

    
    std::cout << "ref7: slot @" << std::hex << &ref7 << " par:" << std::dec << ref7.par1() << ',' << ref7.par2() << '\n';

#ifdef ERR_0
    singleton1 a;  // <- instances of singleton are not allowed 
#endif
#ifdef ERR_1
    singleton1 b ( const_cast<const singleton1 &>(singleton1::instance()) ); // <- instances are not copyable
#endif
#ifdef ERR_2
    singleton1 b;
    b = const_cast<const singleton1 &>(singleton1::instance()); // <- instances are not copyable
#endif

    return 0;
}
