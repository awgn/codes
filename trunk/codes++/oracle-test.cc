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

oracle_std_swap(hello);

struct world : public more::oracle<world>
{
    world()
    {}

    world(int n)
    : oracle_base(n)
    {}

    world(int n, int m)
    : oracle_base(n,m)
    {}

};

oracle_std_swap(world);

int
main(int argc, char *argv[])
try
{    
    more::oracle_trace<hello>::enable();
    more::oracle_trace<world>::enable();

    {
        hello x;
        hello y;

        swap(x,y);
        x = y;

        hello z(x);
    }

    hello abc,x,y;

    char buf[16];

    world * p = new world(10,20); 
    world * q = new (buf) world();

    delete p; 
    delete q;

    return 0;
}
catch(std::exception &e)
{
    std::cout << "std::runtime_error().what() = " << e.what() << std::endl;
}
