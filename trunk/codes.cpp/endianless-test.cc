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
#include <endianless.hh>

using namespace more;

int main()
{
    endianless<short int> x;
    x = 1;

    const endianless<int> y(x);

    endianless<long int> z;

    z = y;

    std::cout << "[host_byte_order]\n";

    std::cout << "   x=" << x << std::endl;
    std::cout << "   y=" << y << std::endl;
    std::cout << "   z=" << z << std::endl;

    std::cout << "[network_byte_order]\n";

    std::cout << "   x=" << x.get() << std::endl;
    std::cout << "   y=" << y.get() << std::endl;
    std::cout << "   z=" << z.get() << std::endl;

}
