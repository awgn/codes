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
#include <proxy.hh>

void copy(int h)
{}
void const_copy(const int h)
{}
void ref(int &h)
{}
void const_ref(const int &h)
{}

using namespace more;

void fun(ref_proxy<int> ref, int n)
{
    std::cout << "      : fun(): ref_proxy<int> ref_value: " << ref.get() << ". set reference to -> " << n << std::endl;
    ref.get() = n;
}

int main(int argc, char *argv[])
{
    int a = 0;
    int b = 1;

    write_enable<int>   val(a);
    write_enable<int,1> val_1(b);

    // val = 1;                     // not allowed
    // val = val_1;                 // not allowed

    val = write_enable<int>(1);     // OK: write enabled
    val = write_enable<int>(val_1); // OK: write enabled

    // int * p = &val;              // not allowed
    const int * p = &val;           // OK 

    // int & r = val;               // not allowed
    const int & r = val;            // OK 

    // ref(val);                    // not allowed
    const_ref(val);                 // OK

    copy(val);                      // OK
    const_copy(val);                // OK

    int c = val;                    // copy is enabled
    const int d = val;              // copy is enabled

    std::cout << "write_enable<int> val: " << val << std::endl;
    
    // ref_proxy at work:

    ref_proxy<int> hello(10);
    std::cout << "ref_proxy<int> hello : " << hello.get() << std::endl;
    
    int xxx = 0;
    ref_proxy<int> world(xxx);

    world.get() = 69;

    std::cout << "ref_proxy<int> world  : " << world.get() << std::endl;
    std::cout << "               ref_val: " << world.get() << std::endl;

    std::cout << "fun() test:" << std::endl;
    fun( ref_proxy<int>(0) , 0);

    int yyy = 1;
    std::cout << "before: yyy=" << yyy << std::endl;
    fun( ref_proxy<int>(yyy), 12345 );
    std::cout << "after : yyy=" << yyy << std::endl;

    int zzz = 2;
    std::cout << "before: zzz=" << zzz << std::endl;
    fun( ref_proxy<int>(std::tr1::ref(zzz)), 12345 );
    std::cout << "after : zzz=" << zzz << std::endl;

    return 0;
}
