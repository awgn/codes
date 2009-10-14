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
#include <any.hh>

int
main(int argc, char *argv[])
{
    more::any a(1);
    const more::any b(std::string("hello world!"));

    more::any c; c = b;
    more::any d(c);

    std::cout << "int a = " << more::any_cast<int>(a) << std::endl;
    
    std::cout << more::any_cast<std::string>(b) << std::endl;
    std::cout << more::any_cast<const std::string>(b) << std::endl;
    
    std::cout << * more::any_cast<std::string>(&c) << std::endl;

    std::cout << std::boolalpha << "a.empty() = " << a.empty() << std::endl;
    std::cout << std::boolalpha << "more::any().empty() = " << more::any().empty() << std::endl;

    more::any_out a_out(10);
    more::any_out b_out(std::string("Hello world!") );

    std::cout << a_out << std::endl;
    std::cout << b_out << std::endl;

    std::cout << more::any_cast<std::string>  ( b_out.get() ) << std::endl;
    std::cout << * more::any_cast<std::string>(&b_out.get() ) << std::endl;

    return 0;
}
 

