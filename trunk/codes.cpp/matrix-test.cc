/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include "matrix.hh"

int main()
{
        more::Matrix<int> a(2,2);
        more::Matrix<int> b(2,2);

        a(0,0) = 1;
        a(0,1) = 2;
        a(1,0) = 3;
        a(1,1) = 4;

        b = a;
        more::Matrix<int> c(a);

        std::cout << a << b << c ; 
}
