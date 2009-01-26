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

struct hello1: protected generic::singleton<hello1> 
{
    SINGLETON_CTOR(hello1) {}
};


class hello2 : protected generic::singleton<hello2> 
{
    int __par1;
    int __par2;

public:

    SINGLETON_CTOR(hello2), __par1(0), __par2(1) {}

    const int par1() const 
    { return __par1; }

    const int par2() const 
    { return __par2; }
};

class hello3 : protected generic::singleton<hello3, true /* volatile singleton */ > {

    int __par1;
    int __par2;

public:
    VOLATILE_SINGLETON_CTOR(hello3, int _par1, int _par2 = 0 ), __par1(_par1), __par2(_par2) {}

    const int par1() const volatile 
    { return __par1; }

    const int par2() const volatile 
    { return __par2; }
};


int 
main(int argc, char *argv[])
{
    hello1 &ref1 = hello1::instance();
    hello2 &ref2 = hello2::instance();
    volatile hello3 &ref3 = hello3::instance<int,int>(0,1);           // singleton: slot signature <int,int> 

    std::cout << "\nsingleton instance (via volatile reference)...\n";
    std::cout << "note: volatile is a contract. The class writer must honour it,\n";
    std::cout << "      by writing volatile methods that are thread-safe.\n\n";

    std::cout << "ref1 -> singleton instance @" << std::hex << &ref1  << '\n';
    std::cout << "ref2 -> singleton instance @" << std::hex << &ref2  << '\n';
    std::cout << "ref3 -> singleton instance @" << std::hex << &ref3  << '\n';

    volatile hello3 &ref4 = hello3::instance<int>(1);               // singleton: slot <int> 

    std::cout << "\n1 singleton: slot <int,int> | slot <int>\n";

    std::cout << "ref3: slot @" << std::hex << &ref3 << " par:" << std::dec << ref3.par1() << ',' << ref3.par2() << '\n';
    std::cout << "ref4: slot @" << std::hex << &ref4 << " par:" << std::dec << ref4.par1() << ',' << ref4.par2() << '\n';

    volatile hello3 & ref5 = hello3::instance<int,int>();
    volatile hello3 & ref6 = hello3::instance<int>();

    std::cout << "ref5: slot @" << std::hex << &ref5 << " par:" << std::dec << ref5.par1() << ',' << ref5.par2() << '\n';
    std::cout << "ref6: slot @" << std::hex << &ref6 << " par:" << std::dec << ref6.par1() << ',' << ref6.par2() << '\n';

#ifdef ERR_0
    hello1 a;  // <- instances of singleton are not allowed 
#endif
#ifdef ERR_1
    hello1 b ( const_cast<const hello1 &>(hello1::instance()) ); // <- instances are not copyable
#endif
#ifdef ERR_2
    hello1 b;
    b = const_cast<const hello1 &>(hello1::instance()); // <- instances are not copyable
#endif

    return 0;
}
