/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
   
#include <print.hpp>
#include <thread>
#include <cstdio>
#include <chrono>

#include <boost/format.hpp>

int
main(int argc, char *argv[])
{
    more::print(std::cout, "%1 %2\n", "hello", std::string("world"));
    more::print(std::cout, "0x%1\n", more::flags<std::ios::hex>(3735928559));
    more::print(std::cout, "%1\n", std::thread::id());

    char buffer[4] = { 'a', 'a', 'a', 'a' };

    more::bprint(buffer,4, "%1!", 42);
    std::cout << buffer << std::endl;


    std::string xxx("world");

    std::cout << "performance test: " << std::flush;

    auto tstart = std::chrono::system_clock::now();
    char tmp[1024];

    for(int i=0; i < 10000000; i++)
    {
        // sprintf(tmp, "%s %s %d\n", "hello", xxx.c_str(), 19230983);
        more::bprint(tmp,1024,"%1 %2 %3\n", "hello", xxx, 19230983);
        // more::sprint("%1 %2 %3\n", "hello", xxx, 19230983);
        // boost::format("%1% %2% %3%\n") % "hello" % xxx % 19230983;
    }
    
    auto diff = std::chrono::system_clock::now() - tstart;
    
    std::cout << static_cast<double>(static_cast<std::chrono::microseconds>(diff).count())/1000000 << " sec." << std::endl;
    return 0;
}
 
