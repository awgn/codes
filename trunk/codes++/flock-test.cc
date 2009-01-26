/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <flock.hh>
#include <cstdlib>

int main()
{
    std::cout << "test: LOCK_EX...\n";
    {
        more::scoped_flock<LOCK_EX|LOCK_NB> l("/tmp/test-flock");
        if (!l.is_open()) {
            std::cout << "open;" << strerror(errno) << std::endl;
            exit(1);
        }
        if (!l.is_locked()) {
            std::cout << "scoped_flock: " << strerror(errno) << std::endl;
            exit(1);
        }

        std::cout << "locked, sleeping...\n";
        sleep(10);
        std::cout << "unlocking...\n";
    }
    std::cout << "done\n";
}
