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
#include <fstream>
#include <sstream>
#include <vector>

#include <iterator>
#include <algorithm>

#include <token.hh>

int
main(int argc, char *argv[])
{
    std::string abc("one:two:three");

    {
        std::istringstream in(abc);

        more::token_string<':'> one;
        more::token_string<':'> two;
        more::token_string<':'> three;

        in >> one >> two >> three;

        const std::string & r_three = three;

        std::cout << static_cast<std::string>(one) << ' ' << two.get() << ' ' << r_three << " done!" << std::endl; 
    }

    {
        std::istringstream in(abc);
        std::vector<std::string> vec;

        std::copy( std::istream_iterator< more::token_string<':'> >(in),
                   std::istream_iterator< more::token_string<':'> >(),
                   std::back_inserter(vec) );

        std::copy( vec.begin(), vec.end(), std::ostream_iterator<std::string>(std::cout, " "));
        std::cout << "done!" << std::endl;
    }

    return 0;
}
 
