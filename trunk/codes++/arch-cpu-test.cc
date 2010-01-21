/* $Id$*/
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <arch-cpu.hh>

#include <algorithm>
#include <iterator>

int
main(int argc, char *argv[])
{
    more::arch::cpu cpu0(0);
    std::cout << "tsc: " << more::arch::cpu::get_cycles() << std::endl;

    std::tr1::shared_ptr<const cpufreq_policy> p = cpu0.policy();

    if (p) {
        std::cout << "min: " << p->min << std::endl;
        std::cout << "max: " << p->max << std::endl;
        std::cout << "gov: " << p->governor << std::endl;
    }

    std::cout << "governors: ";
    const std::list<std::string> & govs = cpu0.available_governors();
    std::copy(govs.begin(), govs.end(), std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "frequencies: ";
    const std::list<unsigned long> & freq = cpu0.available_frequencies();
    std::copy(freq.begin(), freq.end(), std::ostream_iterator<unsigned long>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "setting governor 'performance'..." << std::endl;
    cpu0.set_policy_governor("performance");

    return 0;
}
 
