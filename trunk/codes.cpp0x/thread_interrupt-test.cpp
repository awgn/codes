/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#include <thread>
#include <iostream>
#include <functional>
#include <chrono>
#include <thread>

#include <thread_interrupt.hpp>

using namespace more;

////////////////////////////////////////////////////////
// flavor one: self registering thread

struct thread_one : public std::unary_function<int, void>
{
    void
    operator()(int n) const
    {
        thread_interrupt::hook interrupt_requested;

        for(;;)
        {
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
            if ( interrupt_requested() ) {
                std::cout << "interrupted! " << std::this_thread::get_id() << std::endl;
                return;
            }                                        
        }
    }
};

////////////////////////////////////////////////////////
// flavor two: thread registered by the factory 
// make_interruptible_thread().
 
struct thread_two : public std::binary_function<int, more::thread_interrupt::hook, void>
{
    void
    operator()(int n, more::thread_interrupt::hook interrupt_requested) const
    {
        for(;;)
        {
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
            if ( interrupt_requested() ) {
                std::cout << "interrupted! " << std::this_thread::get_id() << std::endl;
                return;
            }                                        
        }
    }
};
 

  int
main(int argc, char *argv[])
{
    std::thread one(thread_one(), 42);

    std::thread two = make_interruptible_thread(thread_two(), 42);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    thread_interrupt::interrupt(one.get_id());

    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    thread_interrupt::interrupt(two.get_id());
    
    one.join();
    two.join();

    return 0;
}
 
