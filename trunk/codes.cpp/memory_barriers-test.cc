 /* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <memory_barriers.hh>

int
main(int argc, char *argv[])
{
    more::memory_barrier();
    more::memory_read_barrier();
    more::memory_write_barrier();
    return 0;
}
 
