/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <cassert>
#include <nullptr.hpp>
#include <yats.hpp>
using namespace yats;

Context(more_nullptr_test)
{
    Test(simple)
    {
        char * a = nullptr;
        Assert( a == nullptr , is_true());
        Assert( nullptr == a , is_true());
    }

}

int
main(int argc, char *argv[])
{
    return yats::run();
}

