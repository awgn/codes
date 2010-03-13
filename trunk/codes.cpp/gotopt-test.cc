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
#include <vector>
#include <string>
#include <map>

#include <algorithm>
#include <iterator>

#include <gotopt.hh>     // more!

using namespace more::gotopt;

struct option opts[] = 
{
    option("Section:"),
    option(+'l', "hello", 2, "this option accepts 2 integers"),
    option(-'w', "world", 0, "world option"),
    option(),
    option("General:"),
    option(+'a', NULL ,   0),
    option(+'b', NULL ,   0, "this is the b options"),
    option(+'h', "help" , 0, "print this help.")
};

int
main(int argc, char *argv[])
{
    parser par(argv+1, argv+argc, opts);

    char o;
    while( (o = par(make_tuple(
                               make_pair( _l >> _w, "--hello -> --world error!"),
                               make_pair( _a % _b,  "-a and -b are in mutual exclusion!"), // rule with custom error
                               _a || _b  // simple rule...
                               )
          )))
        switch(o)
        {
        case 'a': break;
        case 'b': break;
        case 'l': 
            par.optarg<int>();
            par.optarg<int>();
            std::cout << "--hello given!" << std::endl; 
            break;

        case 'w': 
            std::cout << "--world given!" << std::endl;
            break;

        case 'h': 
            std::cout << more::gotopt::usage("Gotpot class test.\nUsage: a.out [OPTION]...", opts, "Done.");
            return 0;
        }
    
    std::cout << "optind = " << par.optind() << std::endl;

    std::cout << "residual: ";
    std::copy( par.current(), par.end(), std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << std::endl;

    return 0;
}

