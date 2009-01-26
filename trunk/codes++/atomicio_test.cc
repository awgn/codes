/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include "atomicio.hh"

int main()
{
    char buffer[4];
    more::atomicio<>(write, 1, "hello world!\n",13);
    more::atomicio<>(read, 0, buffer, 4);

}
