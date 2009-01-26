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
#include <string>
#include <syslog.hh>

extern char *__progname;

int main(int argc, char **argv)
{
    sys::ctrl(sys::log)->open("sys::log");
    sys::log << "1 - hello " << "world!" << std::endl;
    
    sys::ctrl(sys::plog)->open("sys::plog");
    sys::plog << "2 - hello world!" << std::endl;

    sys::ctrl(sys::err)->open("sys::err");
    sys::err << "3 - hello world!" << std::endl;

    sys::ctrl(sys::perr)->open("sys::perr");
    sys::perr << "4 - hello world!" << std::endl;

    sys::ctrl(sys::perr)->setlevel(LOG_EMERG);
    sys::perr << "5 - emergency message!!!" << std::endl;

}
