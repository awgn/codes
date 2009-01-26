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
//#include <boost/thread.hpp>
#include "atomic.hh"

using namespace more;

typedef atomicity::GNU_CXX_RECURSIVE AtomicityPolicy;

// this bad_class is shared between threads.
//

class bad_class
{
    int val;

public:
    bad_class() 
    :
    val(0)
    {}

    bad_class(int i) : 
    val(i) 
    {}

    bad_class &operator++(int) volatile 
    {
        val++;
        return *const_cast<bad_class *>(this);
    }
    bad_class &operator--(int) volatile
    {
        val--;
        return *const_cast<bad_class *>(this);
    }

    int get() volatile const { return val; }
};

// good_class is also shared between threads, but
// it correctly derives from atomic_class...
//

class good_class : public more::atomic_class<AtomicityPolicy>
{
    int val;

public:
    good_class() : 
    val(0)
    {}
    good_class(int i) : val(i) {}

    good_class &operator++(int) 
    {
        val++;
        return *this;
    }
    good_class &operator--(int) 
    {
        val--;
        return *this;
    }

    int get() volatile const { return val; }
};

// shared instances must be volatile 
//

volatile bad_class   example_0(0);
volatile good_class  example_1(0);
volatile atomic_word<int> example_2(0);

// threads..
//

void *thread_0(void *)
{
    for (int i=0;i<1000000; i++) {
        example_0++;
    }
    return NULL;
}

void *thread_1(void *)
{
    for (int i=0;i<1000000; i++) {
        example_0--;
    }
    return NULL;
}

void *thread_2(void *)
{
    for (int i=0;i<1000000; i++) {
        volatile_ptr<good_class, AtomicityPolicy> ptr(example_1);
        (*ptr)++;
    }
    return NULL;
}

void *thread_3(void *)
{
    for (int i=0;i<1000000; i++) {
        // a capillary way to run a single
        // method (thread safe) is using a temporary...
        (*volatile_ptr<good_class, AtomicityPolicy >(example_1))--;
    }
    return NULL;
}

void *thread_4(void *)
{
    for (int i=0;i<1000000; i++) {
        example_2++;
    }
    return NULL;
}

void *thread_5(void *)
{
    for (int i=0;i<1000000; i++) {
        example_2--;
    }
    return NULL;
}

const inline char *check(int n)
{
    if (n == 0)
        return " [ok]";
    return " [failed!]";
}


int main()
{
    pthread_t a;
    pthread_t b;

    std::cout << "\ndirect access test (bad): " << std::endl;
    for (int i=0; i < 20 ; i++) {
        pthread_create(&a, NULL, thread_0, NULL); 
        pthread_create(&b, NULL, thread_1, NULL); 

        pthread_join(a,NULL);
        pthread_join(b,NULL);

        int n = example_0.get();

        std::cout << "example_0: " << n << check(n) << std::endl;
    }

    std::cout << "\nvolatile_ptr<> access test: " << std::endl;
    for (int i=0; i < 20 ; i++) {
        pthread_create(&a, NULL, thread_2, NULL); 
        pthread_create(&b, NULL, thread_3, NULL); 

        pthread_join(a,NULL);
        pthread_join(b,NULL);

        int n = example_1.get();

        std::cout << "example_1: " << n << check(n) << std::endl;
    }

    std::cout << "\natomic_word test:" << std::endl;
    for (int i=0; i < 20 ; i++) {
        pthread_create(&a, NULL, thread_4, NULL); 
        pthread_create(&b, NULL, thread_5, NULL); 

        pthread_join(a,NULL);
        pthread_join(b,NULL);

        int n = static_cast<int>(example_2);    
        std::cout << "example_2: " << n << check(n) << std::endl;
    }

    std::cout << "\nDo you wonder why atomic_word increments/decrements are exactly 3 times faster? :)" << std::endl;
}

