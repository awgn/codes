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
#include <lnisteambuf.hh>

int
main(int argc, char *argv[])
{
    std::stringstream sstr("this is\n a test\nhello world!");

    std::istream in( new more::lnistreambuf(sstr.rdbuf()) );

    std::string s; 
    while( in >> s)
    {
        std::cout << more::lineno(in) << ": " << s << std::endl;
    }

    return 0;
}
 
