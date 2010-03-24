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

#include <shared_queue.hh>
#include <cassert>

class elem 
{
    int _M_elem;
public:

    elem (int _e) 
    : _M_elem(_e)
    {}

    ~elem()
    {}

    int value()
    { 
        return _M_elem; 
    }
};

more::shared_queue<elem *,1024,1,3> queue;    

void *thread_producer(void *)
{
    for(int i=0; i<100000; ) {

        elem * p = new elem(i);
retry:
        if ( queue.push_back(p) ) {
            std::cout << "push: value->(" << i << ")" << std::endl; 
            i++;
            usleep(1000);
            continue;
        }
        usleep(1000);
        goto retry;
    }

    return NULL;
}

void *thread_consumer(void *)
{
    elem *r;
    for(;;) {
        if (!queue.pop_front(r) )
        {
            sched_yield();
            continue;
        }

        assert(r);
        std::cout << "pop: " << r->value() <<  " (size = " << queue.size() << ")" <<std::endl;
        delete r;
        usleep(5000);
    }

    return NULL;
}

int main()
{
    pthread_t a;
    pthread_t b;
    pthread_t c;
    pthread_t d;

    pthread_create(&a, NULL, thread_producer, NULL);
    pthread_create(&b, NULL, thread_consumer, NULL);
    pthread_create(&c, NULL, thread_consumer, NULL);
    pthread_create(&c, NULL, thread_consumer, NULL);

    pthread_join(a,NULL);
    pthread_join(b,NULL);
    pthread_join(c,NULL);
    pthread_join(d,NULL);
}
