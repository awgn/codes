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
#include <sstream>

#include <iomanip.hh>

int
main(int argc, char *argv[])
{
    std::string str("this line is ignored\none:two: three \nfour");
    std::istringstream sstr(str);
        
    sstr >> more::ignore_line;  // <- ignore_line

    more::token_string tok("\n: "); // <- token_string
    while ( sstr >> tok)
    {
        std::cout << more::spaces(4) << "token_string: [" << tok.str() << "]" << std::endl;  // <- spaces
    }

    return 0;
}
 
