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

    out.async([](std::ostream &)
             {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                throw std::runtime_error("exception thrown in the log closure!");
             });

    out.async([](std::ostream &o)
             {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                o << "Hello world!" << std::endl;
             });


    out.sync([=](std::ostream &o)
             {
                o << 'a' << ' ' << 'b' << ' ' << 'c' << '!' << std::endl;
             });

    // using operator<< log is synchronous through the temporary lazy_ostream ...
    //

    out << "Hello" << " world " << 42 << '!' << std::endl;

}

 
