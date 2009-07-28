/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <atomicio.hh>

using namespace more;

int main()
{
    // atomic_io policy:
    //

    more::atomic_io::call(write, 1, "hello world!\n",13);

    // io functors:
    //

    char buffer[8];

    more::functor_read<>  r0(0, buffer, 4); r0();
    more::functor_write<> w0(1, buffer, 4); w0();

    more::functor_read<atomic_io>  r1(0, buffer, 4); r1();
    more::functor_write<atomic_io> w1(1, buffer, 4); w1();
}
