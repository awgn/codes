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
#include <mtp.hh>

template <typename T>
typename mtp::enable_if< traits::is_class<T>::value ,int>::type 
fun(const T &v) 
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}

template <typename T>
typename mtp::disable_if< traits::is_class<T>::value ,int>::type 
fun(T v) 
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    return 0;
}

struct object { };

int main()
{
    int a = 0;
    object b;

    fun(a);
    fun(b); 
}
