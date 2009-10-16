/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <mutex.hh>

int
main(int, char *[])
{
    more::mutex::type m1;
    more::mutex::scoped_lock l1(m1);

    more::recursive_mutex::type m2;
    more::recursive_mutex::scoped_lock l2_a(m2);
    more::recursive_mutex::scoped_lock l2_b(m2);

    return 0;
}
 
