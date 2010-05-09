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

#include <thread_interrupt.hpp>

using namespace more;

struct test
{
    void
    operator()() const
    {
        for(;;)
        {
            std::cout << __PRETTY_FUNCTION__ << std::endl;
            if ( that_thread::interruption_requested() ) {
                return;
            }                                        
        }
    }
};

  int
main(int argc, char *argv[])
{
    std::thread abc( (test()) );

    sleep(1);

    that_thread::interrupt( abc.get_id() );
    abc.join();

    return 0;
}
 
