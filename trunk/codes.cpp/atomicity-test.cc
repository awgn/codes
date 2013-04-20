/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 *
 */

#include <atomicity-policy.hh>
#include <iostream>

///////////////////////////////////////////////
// compile with and without -pthread option...

using namespace more;

struct test : private atomicity::emptybase_mutex<atomicity::DEFAULT>
{
    int n;

    void method()
    {
        atomicity::DEFAULT::scoped_lock L(this->mutex());
    }
};


int
main(int, char *[])
{
    std::cout << sizeof(test) << std::endl;
    return 0;
}
 
