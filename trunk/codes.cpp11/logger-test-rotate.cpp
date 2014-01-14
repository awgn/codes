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
main(int, char *[])
{
    more::logger out;

    out.open("logger.txt");

    for(int i = 0; i < 1000; ++i)
    {
        out.sync([=](std::ostream &out)
             {
                out << "Hello world!" << std::endl;
             });

        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        out.rotate_async(5, 10);
    }
}

 
