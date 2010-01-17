/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

/* this is a simple source code for testing yaccp iteartors at work */

#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>

// c++ comment 

int a; // c++ comment 
int b; // c++ comment /* void 

int c; /*** // this is 
a C comment... 
           **/

const char * str = "this is /* a string */ \" literal";

int d;/** **/int e;/**/int f;

int g;/*****

#ifndef GOOFY 
#define GOOFY 

#endif 

*******/int h;

#define hello() {\
       hello();\
   }

int
main(int argc, char *argv[])
{
    std::copy(argv,argv+argc, std::ostream_iterator<std::string>(std::cout, " "));
    std::cout << std::endl;
    return 0;
}

