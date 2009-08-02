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

#include <timer.hh>
#include <signal.h>

using namespace more::time;

more::time::tick_type global_tick; // external linkage

template <tick_type & Tc> 
struct Thread_1 : public more::posix::thread, private ticker<Tc, itimer_trait<ITIMER_REAL>::signal_expiration > 
{    
    Thread_1()
    {}

    ~Thread_1()
    {
        this->cancel();
    }

    void *operator()()
    {
        uint32_t ts;
        for(int i=0; i < 20; ++i) {
            this->wait_for_tick(ts);  // timer

            std::cout << std::hex << 
            "ITIMER_REAL: [" << this->get_id() << "] " << std::dec << i << " << tick = {" << std::dec << Tc.current << "}\n";
        }

        return NULL;
    }
};


template <tick_type & Tc> 
struct Thread_2 : public more::posix::thread, private ticker<Tc, SIGRT_MIN > 
{    
    Thread_2()
    {}

    ~Thread_2()
    {
        this->cancel();
    }

    void *operator()()
    {
        uint32_t ts;
        for(int i=0; i < 20; ++i) {
            this->wait_for_tick(ts);  // timer

            std::cout << std::hex << 
            "SIGRT_MIN: [" << this->get_id() << "] " << std::dec << i << " << tick = {" << std::dec << Tc.current << "}\n";
        }

        return NULL;
    }
};

template <tick_type & Tc> 
struct Thread_3 : public more::posix::thread, private ticker<Tc, SIGRT_MIN+1 > 
{    
    Thread_3()
    {}

    ~Thread_3()
    {
        this->cancel();
    }

    void *operator()()
    {
        uint32_t ts;
        for(int i=0; i < 20; ++i) {
            this->wait_for_tick(ts);  // timer

            std::cout << std::hex << 
            "SIGRT_MIN+1: [" << this->get_id() << "] " << std::dec << i << " << tick = {" << std::dec << Tc.current << "}\n";
        }

        return NULL;
    }
};



using namespace more::posix;

int
main(int argc, char *argv[])
{    
    ////////////////////////////////////
    // 1) block sigalarm in this thread 

    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGALRM);
    sigaddset(&block, SIGVTALRM);
    sigaddset(&block, SIGPROF);

    sigaddset(&block, SIGRT_MIN);
    sigaddset(&block, SIGRT_MIN+1);

    this_thread::sig_mask(SIG_BLOCK, &block, NULL);

    ///////////////////////////////
    // 2) create the itimer thread

    timeval t = { 0, 10000 };
    concrete_thread< more::time::itimer_pulse_thread<ITIMER_REAL,    global_tick > > x(&t);

    ///////////////////
    // and start it ... 
    x.start();

    //////////////////////
    // 3) create threads

    more::posix::thread * p = new concrete_thread< Thread_1<global_tick> >;
    p->start(); 
    p->join();


    //
    // REALTIME timers
    //
 
    itimerspec spec, spec2;

    spec.it_value.tv_sec  = 0;
    spec.it_value.tv_nsec = 100000000;
    spec.it_interval.tv_sec  = spec.it_value.tv_sec;
    spec.it_interval.tv_nsec = spec.it_value.tv_nsec;

    spec2.it_value.tv_sec  = 0;
    spec2.it_value.tv_nsec = 200000000;
    spec2.it_interval.tv_sec  = spec2.it_value.tv_sec;
    spec2.it_interval.tv_nsec = spec2.it_value.tv_nsec;

  
    {
        std::cout << "simple rt_timer test: " << std::endl;

        more::time::rt_timer<CLOCK_REALTIME, SIGEV_SIGNAL, SIGRT_MIN   > x;
        more::time::rt_timer<CLOCK_REALTIME, SIGEV_SIGNAL, SIGRT_MIN+1 > y;

        x.set(&spec);
        y.set(&spec2);

        sleep(1);

        std::cout << "done." << std::endl;
    }

    {
        std::cout << "rt_timer_thread test: " << std::endl;

        concrete_thread< more::time::rt_timer_pulse_thread<SIGRT_MIN,   global_tick > > rt_1(&spec); 
        concrete_thread< more::time::rt_timer_pulse_thread<SIGRT_MIN+1, global_tick > > rt_2(&spec2); 
        
        rt_1.start();
        rt_2.start();

        more::posix::thread * p = new concrete_thread< Thread_2<global_tick> >;
        more::posix::thread * q = new concrete_thread< Thread_3<global_tick> >;

        p->start(); 
        q->start(); 

        q->join();
        p->join();

        std::cout << "done." << std::endl;
    }

    return 0;
}
 
