/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <nullstreambuf.hh>

int
main(int argc, char *argv[])
{
    std::ostream null(new more::nullstreambuf);

    for(int i=0; i<1000000; i++)
    {
        null << "this string is sent to null";
    }

    return 0;
}
