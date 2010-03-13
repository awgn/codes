/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <pthread.h>
#include "fast_deque.hh"

class elem 
{
    int e;

public:
    elem (int _e) 
    : e(_e)
    {}

    ~elem()
    { std::cout << __PRETTY_FUNCTION__ << std::endl; }

    int value()
    { return e; }
};


volatile more::fast_deque<elem *> deck(100);    


void *thread_a(void *)
{
    int r;
    for(int i=1 ;i<200;) {
        elem * p = new elem(i);
        std::cout << "push: " << i << " " << (r=deck.push_back(p)) << std::endl; 
        if (r >= 0)
            i++;
        usleep(1000);
    }

    return NULL;
}

void *thread_b(void *)
{
    elem *r;
    for(;;) {
        if ( deck.pop_front(r) >= 0 ) {
            std::cout << "pop: " << r->value() << std::endl;
            delete r;
        }
        usleep(100);
        deck.dump();
    }

    return NULL;
}

void *thread_c(void *)
{
    for(int i=0; i<200; i++)
    {
        deck.clear();
        std::cout << "clear: " << deck.empty() << std::endl;       
        usleep(2000);
    }
    return NULL;
}


int main()
{
    pthread_t a;
    pthread_t b;
    // pthread_t c;

    pthread_create(&a, NULL, thread_a, NULL);
    pthread_create(&b, NULL, thread_b, NULL);
    // pthread_create(&c, NULL, thread_c, NULL);

    pthread_join(a,NULL);
    pthread_join(b,NULL);
    // pthread_join(c,NULL);
}
