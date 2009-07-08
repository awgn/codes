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
#include <tqueue.hh>

more::tqueue<int> Q;

struct predicate
{
    bool operator()() const
    {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
        return true;
    }
};

struct action
{
    void operator()() const
    {
        std::cout << __PRETTY_FUNCTION__  << std::endl;
    }
};


struct thread_push : public more::posix::thread
{
    ~thread_push()
    {
        this->cancel();
    }

    void *operator()()
    {
        for(int i = 0; i < 10; i++) {
            Q.push_enabled_if(predicate(), i);
            sleep(1);
        }
        return NULL;
    }

};

struct thread_pop : public more::posix::thread
{
    ~thread_pop()
    {
        this->cancel();
    }

    void *operator()()
    {        
        int n; 
        for(int i = 0; i < 10; i++) {
            Q.pop_and_exec(action(), n);
            std::cout << n << std::endl;
        }
        return NULL; 
    }
};


int
main(int, char *[])
{
    thread_push producer;
    thread_pop  consumer;

    producer.start();
    consumer.start();

    producer.join();
    consumer.join();

    return 0;
}
    
