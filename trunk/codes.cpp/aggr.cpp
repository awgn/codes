/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <lexical_cast.hh>

#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <string>

int
main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "aggreate: winsize (sec)" << std::endl;
        exit(1);
    }

    const long double win = more::lexical_cast<long double>( std::string(argv[1]) );

    long double timestamp, t0;
    long double value;
    long double aggr = 0.0;

    std::cout << std::fixed;
    std::cin  >> t0;
    
    timestamp = t0;

    while( std::cin >> value )
    {
        if ( (timestamp - t0) <  win )
        {
            aggr += value;
        }
        else 
        {
            do 
            {
                std::cout << (t0+win) << " " << (aggr/win) << std::endl;
                t0  += win;
                aggr = 0.0;
            }
            while(t0+win <= timestamp);

            // reset...
            //
            t0 = timestamp;
            aggr = value;
        }
        
        if( ! (std::cin >> timestamp) )
            break;
    } 
    return 0;
}
 
