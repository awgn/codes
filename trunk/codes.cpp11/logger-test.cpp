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

more::logger<> out;
// more::logger<std::mutex> out;
// more::logger<std::recursive_mutex> out;

int
main(int, char *[])
{
    out.add_header(more::decorator::timestamp);
    out.add_header(more::decorator::thread_id);

    out.async([](std::ostream &)
             {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                throw std::runtime_error("ASYNC_1: throw in the log closure!");
             });

    out.async([](std::ostream &o)
             {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                o << "ASYNC_2: this is a log message..." << std::endl;

                // NOTE: within a sync() or async() closure you should avoid using
                //       the logger object because it causes a dead-lock. Instead you can:
                //
                //       1) Use a more::safe_mutex (the default one) which detects deadlocks throwing exceptions:
                //          This helps you to fix your code, then you can fall back using the std::mutex.
                //
                //       2) use a std::recursive_mutex which prevents dead-lock at a higher cost.
                //
                // out << "This message potentially causes a dead-lock!" << std::endl;
             });


    out.sync([=](std::ostream &o)
             {
                o << 1 << ' ' << 2 << ' ' << 3 << '!' << std::endl;
             });

    // using operator<< log is synchronous through the temporary lazy_ostream ...
    //

    out << more::log_async << "ASYNC_3:" << " Hello" << " world" << '!' << std::endl;

    out << "Hello" << " world" << '!' << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(2));
}


