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

#include <signal.hpp>

int
main(int, char *[])
{
    more::signal_handler::init(
    {
        { SIGQUIT, []() { std::cout << "exiting... " << std::endl; _Exit(0); } }
    }
    );

    for(;;)
    {
        // use signal_handler::get()...
        //
        
        auto sig = more::signal_handler::get();
                                         
        if (sig) {
            std::cout << "signal " << sig << " pending..." << std::endl;
        }
        else  {
            std::cout << "no signal pending..." << std::endl;
        }

        // or signal_handler::may_throw()... (a unix_signal exception)
        // 
        try 
        {
            more::signal_handler::may_throw();
        }
        catch(more::unix_signal &e)
        {
            std::cout << "got signal " << e.signum() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    return 0;
}

