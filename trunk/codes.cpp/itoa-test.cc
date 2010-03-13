/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <sys/time.h>
#include <time.h>

#include "itoa.hh"

int main()
{
      struct timeval tv;
      gettimeofday(&tv,NULL);
       
      std::cout << more::itoa(-1) << std::endl;
      std::cout << more::itoa(-1UL) << std::endl;
      std::cout << more::itoa(0xdeadbeef,"%x") << std::endl;
      std::cout << more::itoa(tv.tv_sec,"%d") << std::endl; 
}


