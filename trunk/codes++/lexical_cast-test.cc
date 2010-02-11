/* $Id: lexical_cast.hh 445 2010-02-10 21:15:08Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <lexical_cast.hh>
#include <boost/lexical_cast.hpp>

#include <timeval.hh>

int
main(int argc, char *argv[])
{
    const int max_iter = 10000000;

    std::cout << max_iter << " number of lexical_cast<int>(): " << std::endl;

    more::Timeval b = more::Timeval::now();

    for(int i=0; i< max_iter; ++i) 
    {
        boost::lexical_cast<int>("123456");
    }

    more::Timeval e = more::Timeval::now();
    
    std::cout << "boost: " << (e-b).to_msec() << " msec" << std::endl;

    b = more::Timeval::now();

    for(int i=0; i< max_iter; ++i) 
    {
        more::lexical_cast<int>("123456");
    }

    e = more::Timeval::now();

    std::cout << "more: " << (e-b).to_msec() << " msec" << std::endl;

    return 0;
}
 
