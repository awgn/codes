/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <itimer.hh>
#include <pthread++.hh>
#include <iostream>

#include <signal.h>

more::tick_type global; // external linkage

template <more::tick_type & Tc> 
struct Thread : public posix::thread, private more::timer<Tc> 
{    
    Thread()
    {}

    ~Thread()
    {
        this->cancel();
    }

    void *operator()()
    {
        uint32_t ts;
        for(int i=0 ;;) {
           
            this->wait_for_tick(ts);  // timer

            std::cout << std::hex << 
            "[" << this->id() << "] " << std::dec << ++i << " << tick = {" << std::dec << Tc.current << "}\n";
        }

        return NULL;
    }

};

int
main(int argc, char *argv[])
{
    // create two threads
    //

    posix::thread * p = new Thread<global>;
    posix::thread * q = new Thread<global>;
  
    p->start(); 
    q->start();

    // set handler fro SIGALRM
    signal(SIGALRM, more::timer<global>::tick );

    // create the tick-timer
    //

    more::itimer<ITIMER_REAL> x;
    timeval t = { 0, 100000 };
    x.set(&t);

    p->join();
    q->join();

    return 0;
}
 
