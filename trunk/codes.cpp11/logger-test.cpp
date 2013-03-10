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
    more::logger out(std::cout);

    out.log([](std::ostream &o){

                std::this_thread::sleep_for(std::chrono::seconds(2));

                o << "Hello world!" << std::endl;

             });

    out.log([](std::ostream &o){

                std::this_thread::sleep_for(std::chrono::seconds(1));
                o << "Ciao mondo!" << std::endl;

             });

    out.log([=](std::ostream &o){

                o << 'a' << ' ' << 'b' << ' ' << 'c' << '!' << std::endl;

             });


    std::cout << "waiting for the logger threads to be finished...\n";

    std::this_thread::sleep_for(std::chrono::seconds(5));
}

 
