/* $Id$*/
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */


#include <algorithm>
#include <iterator>
#include <iostream>

#include <arch-cpu.hh>

int
main(int, char *[])
{
    std::cout << "current tsc: " << more::arch::cpu::get_cycles() << std::endl;
    std::cout << "Hz : " << more::arch::cpu::Hz() << std::endl;

    more::cpufreq(0).set_policy_governor("performance");

    std::cout << "...refresh Hz: " << more::arch::cpu::Hz(0) << std::endl; // recalc
    std::cout << "Hz : " << more::arch::cpu::Hz() << std::endl;

    return 0;
}
 
