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
#include <algorithm>

#include <string>
#include <vector>
#include <deque>

#include <heap.hh>
#include <tuplarr.hh>
#include <arch-cpu.hh>

int
main(int argc, char *argv[])
{
    more::random_access::heap< int, int, std::vector> test1;
    more::random_access::heap< int, int, std::deque > test2;
    more::redblack::heap<int, int> test3;
   
    // load the heaps with 1000 random values:

    srand(0xcafebabe);
    for(int i=0; i < 1000; ++i) {
        int r = rand();
        test1.push(r, 0);
        test2.push(r, 0);
        test3.push(r, 0);
    }

    // test0: just the rand...

    for(int i=0; i < 10000; ++i) 
    {
        arch::cpu::cycles_t a1 = arch::cpu::get_cycles();
        
        rand(); 

        arch::cpu::cycles_t a2 = arch::cpu::get_cycles();
        
        printf("%llu\n", a2-a1);
    }
 

    // test1...

    for(int i=0; i < 10000; ++i) 
    {
        arch::cpu::cycles_t a1 = arch::cpu::get_cycles();
        
        test1.push(rand(), 0);
        std::pair<int, int> v = test1.pop();

        arch::cpu::cycles_t a2 = arch::cpu::get_cycles();
        
        printf("%llu\n", a2-a1);
    }
    
    // test2...
    
    for(int i=0; i < 10000; ++i) 
    {
        arch::cpu::cycles_t a1 = arch::cpu::get_cycles();

        test2.push(rand(), 0);
        std::pair<int, int> v = test2.pop();

        arch::cpu::cycles_t a2 = arch::cpu::get_cycles();

        printf("%llu\n", a2-a1);
    }

    // test3

    for(int i=0; i < 10000; ++i) 
    {
        arch::cpu::cycles_t a1 = arch::cpu::get_cycles();

        test3.push(rand(), 0);
        std::pair<int, int> v = test3.pop();

        arch::cpu::cycles_t a2 = arch::cpu::get_cycles();

        printf("%llu\n", a2-a1);
    }

    return 0;
}

