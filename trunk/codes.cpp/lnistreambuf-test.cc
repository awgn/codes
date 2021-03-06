/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <sstream>
#include <lnistreambuf.hh>

int
main(int, char *[])
{
    std::stringstream str("this is\n a test\nhello world!");
    std::istream in( new more::lnistreambuf(str.rdbuf()) );

    std::string s; 
    while(in >> s)
    {
        std::cout << more::line_number(in) << ": " << s << std::endl;
    }

    return 0;
}
 
