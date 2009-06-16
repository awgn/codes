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
struct Thread : public more::posix::thread, private more::ticker<Tc> 
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
            "[" << this->get_id() << "] " << std::dec << ++i << " << tick = {" << std::dec << Tc.current << "}\n";
        }

        return NULL;
    }

};

int
main(int argc, char *argv[])
{    
    
    /////////////////////
    // 1) block sigalarm 

    more::block_signal(SIGALRM);

    ///////////////////////////////
    // 2) create the itimer thread

    more::itimer<ITIMER_REAL, global > x;
    timeval t = { 0, 10000 };
    x.set(&t);

    ///////////////////
    // and start it... 
    x.start();

    //////////////////////
    // 3) create a thread

    more::posix::thread * p = new Thread<global>;
  
    p->start(); 
    p->join();

    return 0;
}
 
