/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <oracle.hh>

struct hello : public more::oracle<hello>
{};

int
main(int, char *[])
{
    more::oracle_trace<hello>::enable();

    char buffer[64];

    hello * p;
    hello * q;
    hello * x;
    hello * y;

    try {
        p = new hello;
        q = new (buffer) hello;
        x = new hello[10];
        y = new (buffer) hello[10];
    }
    catch(...) {
        std::cout << "caught exception!" << std::endl;
        exit (1);
    }

    delete p;
    delete []x;

    (void)q;
    (void)y;

    return 0;
}
 
