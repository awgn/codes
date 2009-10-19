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
#include <cassert>
#include <boost/thread/mutex.hpp>

#include <mutex.hh>

template <typename M>
void trylock_test(M &m, bool expected, more::details::true_type)
{
    std::cout << "testing try_lock()..." << std::endl;
    bool e;
    assert( (e = m.try_lock()) == expected);
    if (e)
        m.unlock();

}

template <typename M>
void trylock_test(M &, bool, more::details::false_type)
{
    std::cout << "this mutex LACKS try_lock() method!!!" << std::endl;
}

int
main(int, char *[])
{
    more::mutex::type m1;
    more::mutex::scoped_lock l1(m1);

    more::recursive_mutex::type m2;
    more::recursive_mutex::scoped_lock l2_a(m2);
    more::recursive_mutex::scoped_lock l2_b(m2);

    trylock_test(m1, false, more::mutex::has_try_lock());
    trylock_test(m2, true , more::mutex::has_try_lock() );

    std::cout << "test ok!" << std::endl;
    return 0;
}
 
