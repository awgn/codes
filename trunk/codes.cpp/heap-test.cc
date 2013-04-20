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
#include <lexical_cast.hh>

int
main(int, char *argv[])
{
    more::random_access::vector_heap< int, int > test1;
    more::random_access::priority_queue_heap< int, int > test2;
    more::random_access::deque_heap< int, int > test3;
    more::redblack::heap<int, int> test4;
  
    int how_many = more::lexical_cast<int>(argv[1]);

    // load the heaps with 1000 random values:

    srand(0xcafebabe);
    for(int i=0; i < how_many/2; ++i) {
        int r = rand();
        test1.push(r, 0);
        test2.push(r, 0);
        test3.push(r, 0);
        test4.push(r, 0);
    }

    // test0: just the rand...

    // srand(0xdeadbeef);
    // for(int i=0; i < how_many/2; ++i) 
    // {
    //     more::arch::cpu::cycles_type a1 = more::arch::cpu::get_cycles();
    //     
    //     rand(); 

    //     more::arch::cpu::cycles_type a2 = more::arch::cpu::get_cycles();
    //     
    //     printf("%llu\n", a2-a1);
    // }
 

    // test1...

    srand(0xdeadbeef);
    for(int i=0; i < how_many/2; ++i) 
    {
        more::arch::cpu::cycles_type a1 = more::arch::cpu::get_cycles();
        
        test1.push(rand(), 0);
        std::pair<int, int> v = test1.pop();

        more::arch::cpu::cycles_type a2 = more::arch::cpu::get_cycles();
        
        printf("%d\n", v.first );
        printf("%llu\n", a2-a1);
    }
    
    // test2...
    
    srand(0xdeadbeef);
    for(int i=0; i < how_many/2; ++i) 
    {
        more::arch::cpu::cycles_type a1 = more::arch::cpu::get_cycles();

        test2.push(rand(), 0);
        std::pair<int, int> v = test2.pop();

        more::arch::cpu::cycles_type a2 = more::arch::cpu::get_cycles();

        printf("%d\n", v.first );
        printf("%llu\n", a2-a1);
    }

    // test3

    for(int i=0; i < how_many/2; ++i) 
    {
        more::arch::cpu::cycles_type a1 = more::arch::cpu::get_cycles();

        test3.push(rand(), 0);
        std::pair<int, int> v = test3.pop();

        more::arch::cpu::cycles_type a2 = more::arch::cpu::get_cycles();

        printf("%d\n", v.first );
        printf("%llu\n", a2-a1);
    }

    // test4

    for(int i=0; i < how_many/2; ++i) 
    {
        more::arch::cpu::cycles_type a1 = more::arch::cpu::get_cycles();

        test4.push(rand(), 0);
        std::pair<int, int> v = test4.pop();

        more::arch::cpu::cycles_type a2 = more::arch::cpu::get_cycles();

        printf("%d\n", v.first );
        printf("%llu\n", a2-a1);
    }


    return 0;
}

