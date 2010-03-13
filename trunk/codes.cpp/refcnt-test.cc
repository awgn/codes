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
#include <iomanip>

#include "refcnt.hh"

class test_single 
: public more::refcnt<test_single, more::refcnt_policy::SingleThread> { };

class test_multi 
: public more::refcnt<test_multi, more::refcnt_policy::MultiThread> { };

void *thread_single(void *h)
{
    for(int i = 0; i< 100000; ++i) {
        test_single tmp0;
        test_single tmp1;
        test_single tmp2;
        test_single tmp3;
        test_single tmp4;
        test_single tmp5;
        test_single tmp6;
    }
    return NULL;
}

void *thread_multi(void *h)
{
    for(int i = 0; i< 100000; ++i) {
        test_multi tmp0;
        test_multi tmp1;
        test_multi tmp2;
        test_multi tmp3;
        test_multi tmp4;
        test_multi tmp5;
        test_multi tmp6;
    }
    return NULL;
}

const char *check(int n)
{
    if (n == 0)
        return " [ok]";
    return " [failed!]";
}

int main()
{
    pthread_t a;
    pthread_t b;
    pthread_t c;
    pthread_t d;
    pthread_t e;
    pthread_t f;
    pthread_t g;
    
    std::cout << "ERR: test refcounter(SingleThread) with multiple threads... \n";
    std::cout << "     counter should be 0 but it isn't\n"; 

    for (int i=0; i < 20 ; i++) {

        pthread_create(&a, NULL, thread_single, NULL);
        pthread_create(&b, NULL, thread_single, NULL);
        pthread_create(&c, NULL, thread_single, NULL);
        pthread_create(&d, NULL, thread_single, NULL);
        pthread_create(&e, NULL, thread_single, NULL);
        pthread_create(&f, NULL, thread_single, NULL);
        pthread_create(&g, NULL, thread_single, NULL);
        pthread_join(a,NULL);
        pthread_join(b,NULL);
        pthread_join(c,NULL);
        pthread_join(d,NULL);
        pthread_join(e,NULL);
        pthread_join(f,NULL);
        pthread_join(g,NULL);

        int c = test_single::counter();
        std::cout << "     counter() = " << std::setw(4) << c << check(c) << std::endl; 
    }

    std::cout << "TEST: refcounter(MultiThread) with multiple threads... [correct]\n"; 

    for (int i=0; i < 20 ; i++) {

        pthread_create(&a, NULL, thread_multi, NULL);
        pthread_create(&b, NULL, thread_multi, NULL);
        pthread_create(&c, NULL, thread_multi, NULL);
        pthread_create(&d, NULL, thread_multi, NULL);
        pthread_create(&e, NULL, thread_multi, NULL);
        pthread_create(&f, NULL, thread_multi, NULL);
        pthread_create(&g, NULL, thread_multi, NULL);
        pthread_join(a,NULL);
        pthread_join(b,NULL);
        pthread_join(c,NULL);
        pthread_join(d,NULL);
        pthread_join(e,NULL);
        pthread_join(f,NULL);
        pthread_join(g,NULL);

        int c = test_multi::counter();
        std::cout << "     counter() = " << std::setw(4) << c << check(c) << std::endl; 
    }
}


