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

template <char ch>
struct hexToNibble {
    enum { value = (((ch & 0x40) >> 6) | (((ch & 0x40)>>5) & ((ch & 1)<<1))) ^ (ch & 3) | 
                      ch & ((((ch & 8) >> 1) ^ 0x4) & 4 | 8) | 
                    ((ch & 0x40)>>4) & ((ch&1) << 2) & ((ch&2)<<1) | ((ch&0x40) >> 3) };

};

int main()
{
   std::cout << hexToNibble<'0'>::value << std::endl;
   std::cout << hexToNibble<'1'>::value << std::endl;
   std::cout << hexToNibble<'2'>::value << std::endl;
   std::cout << hexToNibble<'3'>::value << std::endl;
   std::cout << hexToNibble<'4'>::value << std::endl;
   std::cout << hexToNibble<'5'>::value << std::endl;
   std::cout << hexToNibble<'6'>::value << std::endl;
   std::cout << hexToNibble<'7'>::value << std::endl;
   std::cout << hexToNibble<'8'>::value << std::endl;
   std::cout << hexToNibble<'9'>::value << std::endl;
   std::cout << hexToNibble<'A'>::value << std::endl;
   std::cout << hexToNibble<'B'>::value << std::endl;
   std::cout << hexToNibble<'C'>::value << std::endl;
   std::cout << hexToNibble<'D'>::value << std::endl;
   std::cout << hexToNibble<'E'>::value << std::endl;
   std::cout << hexToNibble<'F'>::value << std::endl;
}
