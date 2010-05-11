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

#include <thread_interrupt.hpp>

using namespace more;

struct test : public std::unary_function<that_thread::interrupt_request_type, void>
{
    void
    operator()(int n) const
    {
        that_thread::interrupt_hook interrupt_requested;

        for(;;)
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            if ( interrupt_requested ) {
                std::cout << "interrupted!" << std::endl;
                return;
            }                                        
        }
    }
};

  int
main(int argc, char *argv[])
{
    std::thread abc( test(), 42);

    sleep(1);

    that_thread::interrupt( abc.get_id() );
    abc.join();

    return 0;
}
 
