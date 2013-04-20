/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <unistd.h>

#include <timeval.hh>

#include <iostream>
#include <algorithm>
#include <iterator>
#include <vector>

using namespace more;

int
main(int, char *[])
{
    more::Timeval a( Timeval::now() );

    Timeval b;    

    std::cout << "a = " << a << std::endl;    
    std::cout << "b = " << b << std::endl;        

    if(!b) {
        std::cout << "b is not set! (ok)" << std::endl; 
    }
 
    b = a;

    std::cout << "b = " << b << std::endl;        

    Timeval half(0,500000);

    b+= half;

    std::cout << "b = " << b << std::endl;        

    std::cout << "b - 0.5 sec = " << ( b - half ) << std::endl;

    b.update();

    std::cout << "b = " << b << std::endl;        

    usleep(500000);

    Timeval c( Timeval::now() );

    std::cout << "c = " << c << std::endl;        

    std::cout << "diff = " << (c-b) << std::endl;

    std::cout << std::boolalpha << (c == c) << std::endl;

    std::cout << (c > b)  << std::endl;
    std::cout << (c >= b) << std::endl;
    std::cout << (c >= c) << std::endl;

    std::cout << (c != b) << std::endl;    
    
    std::cout << (b < c ) << std::endl;
    std::cout << (b <= c) << std::endl;
    std::cout << (b <= b) << std::endl;

    std::vector<more::Timeval> vec;

    vec.push_back(c);
    vec.push_back(b);

    std::sort(vec.begin(), vec.end());

    std::cout << "vec: ";
    std::copy(vec.begin(), vec.end(), std::ostream_iterator<Timeval>(std::cout, " "));
    std::cout << std::endl;

    std::cout << "b.to_usec() = " << b.to_usec() << std::endl;

    std::cout << "b = " << b << std::endl;        
    std::cout << "c = " << c << std::endl; 
    
    std::swap(b,c);

    std::cout << "b = " << b << std::endl;        
    std::cout << "c = " << c << std::endl; 

    return 0;
}
 
