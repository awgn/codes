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
#include <enable_exception_if.hh>

#ifndef EXCEPTION_ENABLED   
#define EXCEPTION_ENABLED false
#endif

class test_class : public more::enable_exception_if<EXCEPTION_ENABLED> {

public:

    test_class()
    {
        throw_or_return ( std::runtime_error("exception") );
    }

    int hello()
    {
        throw_or_return ( std::runtime_error("exception"), -1 );
    }

};


int
main(int, char *[])
{
    test_class x;

    try {
        if (x) {
            std::cout << "testclass constructor ok!\n";
        }    
        else {
            std::cout << "testclass constructor failed!\n";
        }
    }
    catch(std::exception &e) {
        // throw;
    }

    return 0;
}

