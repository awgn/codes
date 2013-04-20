/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <logger.hpp>

int
main(int argc, char *argv[])
{
    more::logger out;

    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        for(int i = 0; i < 200; i++)
        {
            out.async_ratelimit<5>(std::chrono::system_clock::now(), [](std::ostream &o)
            {
                o << "hello world" << std::endl;
            });
        
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

}

 
