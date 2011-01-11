/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <errcode.h>

#include <iostream>
#include <string>
#include <cassert>

#include <yats.hpp>
using namespace yats;

Context(error_code_class_test)
{
    Test(string_error_code)
    {
        Assert( more::strerrcode(ENOTRECOVERABLE), is_equal_to(std::string("ENOTRECOVERABLE")) );
        Assert( more::gai_strerrcode(EAI_AGAIN),   is_equal_to(std::string("EAI_AGAIN")) );
    } 

    Test(exception)
    {
        Assert_Throw(more::strerrcode(180));
    }
}
 
int
main(int argc, char *argv[])
{
    return yats::run();
}
