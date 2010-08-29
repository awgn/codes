/* $Id: errcode-test.cc 466 2010-03-13 12:31:43Z nicola.bonelli $ */
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

int
main(int argc, char *argv[])
{
    assert( std::string("ENOTRECOVERABLE") == more::strerrcode(ENOTRECOVERABLE) );
    assert( std::string("EAI_AGAIN") == more::gai_strerrcode(EAI_AGAIN) );
    
    try 
    {
        more::strerrcode(180);
    }
    catch(...) 
    {
        std::cout << "ok." << std::endl;
    }

    return 0;
}
 
