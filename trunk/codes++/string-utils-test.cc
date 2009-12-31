/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <string-utils.hh>
#include <iostream>

int
main(int argc, char *argv[])
{
    std::string s0("aB Ab");
    std::string s1("   aB Ab");
    std::string s2("aB Ab  ");
    std::string s3("   aB Ab  ");

    std::cout << "orig: [" << s0 << ']' << std::endl;
    std::cout << "orig: [" << s1 << ']' << std::endl;
    std::cout << "orig: [" << s2 << ']' << std::endl;
    std::cout << "orig: [" << s3 << ']' << std::endl;

    std::cout << std::endl;
    std::cout << "trim: [" << more::trim(s0) << ']' << std::endl;
    std::cout << "trim: [" << more::trim(s1) << ']' << std::endl;
    std::cout << "trim: [" << more::trim(s2) << ']' << std::endl;
    std::cout << "trim: [" << more::trim(s3) << ']' << std::endl;

    std::cout << std::endl;
    std::cout << "left_trim: [" << more::left_trim(s0) << ']' << std::endl;
    std::cout << "left_trim: [" << more::left_trim(s1) << ']' << std::endl;
    std::cout << "left_trim: [" << more::left_trim(s2) << ']' << std::endl;
    std::cout << "left_trim: [" << more::left_trim(s3) << ']' << std::endl;

    std::cout << std::endl;
    std::cout << "right_trim: [" << more::right_trim(s0) << ']' << std::endl;
    std::cout << "right_trim: [" << more::right_trim(s1) << ']' << std::endl;
    std::cout << "right_trim: [" << more::right_trim(s2) << ']' << std::endl;
    std::cout << "right_trim: [" << more::right_trim(s3) << ']' << std::endl;

    std::cout << std::endl;
    std::cout << "swapcase: [" << more::swapcase(s0) << ']' << std::endl;
    std::cout << "swapcase: [" << more::swapcase(s1) << ']' << std::endl;
    std::cout << "swapcase: [" << more::swapcase(s2) << ']' << std::endl;
    std::cout << "swapcase: [" << more::swapcase(s3) << ']' << std::endl;
 
    std::cout << "swapcase: [" << more::swapcase_(s0) <<']' <<  std::endl; // in place version...
    std::cout << "swapcase: [" << more::swapcase_(s1) <<']' <<  std::endl;
    std::cout << "swapcase: [" << more::swapcase_(s2) <<']' <<  std::endl;
    std::cout << "swapcase: [" << more::swapcase_(s3) <<']' <<  std::endl;
 
    std::cout << std::endl;
    std::cout << "upcase: [" << more::upcase(s0) << ']' << std::endl;
    std::cout << "upcase: [" << more::upcase(s1) << ']' << std::endl;
    std::cout << "upcase: [" << more::upcase(s2) << ']' << std::endl;
    std::cout << "upcase: [" << more::upcase(s3) << ']' << std::endl;
 
    std::cout << "upcase: [" << more::upcase_(s0) <<']' <<  std::endl;  // in place version...
    std::cout << "upcase: [" << more::upcase_(s1) <<']' <<  std::endl;
    std::cout << "upcase: [" << more::upcase_(s2) <<']' <<  std::endl;
    std::cout << "upcase: [" << more::upcase_(s3) <<']' <<  std::endl;
  
    std::cout << std::endl;
    std::cout << "downcase: [" << more::downcase(s0) << ']'<< std::endl;
    std::cout << "downcase: [" << more::downcase(s1) << ']'<< std::endl;
    std::cout << "downcase: [" << more::downcase(s2) << ']'<< std::endl;
    std::cout << "downcase: [" << more::downcase(s3) << ']'<< std::endl;
 
    std::cout << "downcase: [" << more::downcase_(s0) <<']'<<  std::endl; // in place version...
    std::cout << "downcase: [" << more::downcase_(s1) <<']'<<  std::endl;
    std::cout << "downcase: [" << more::downcase_(s2) <<']'<<  std::endl;
    std::cout << "downcase: [" << more::downcase_(s3) <<']'<<  std::endl;

    std::cout << std::endl;
    std::cout << "casecmp: " << std::boolalpha << more::casecmp("aB","Ab") << std::endl;

    std::cout << std::endl;
    std::cout << "capitalize: [" << more::capitalize(s0) << ']'<< std::endl;
    std::cout << "capitalize: [" << more::capitalize(s1) << ']'<< std::endl;
    std::cout << "capitalize: [" << more::capitalize(s2) << ']'<< std::endl;
    std::cout << "capitalize: [" << more::capitalize(s3) << ']'<< std::endl;
 
    std::cout << "capitalize: [" << more::capitalize_(s0) << ']'<< std::endl;
    std::cout << "capitalize: [" << more::capitalize_(s1) << ']'<< std::endl;
    std::cout << "capitalize: [" << more::capitalize_(s2) << ']'<< std::endl;
    std::cout << "capitalize: [" << more::capitalize_(s3) << ']'<< std::endl;

    std::cout << std::endl;
    std::cout << "reverse: [" << more::reverse(s0) << ']'<< std::endl;
    std::cout << "reverse: [" << more::reverse(s1) << ']'<< std::endl;
    std::cout << "reverse: [" << more::reverse(s2) << ']'<< std::endl;
    std::cout << "reverse: [" << more::reverse(s3) << ']'<< std::endl;
 
    std::cout << "reverse: [" << more::reverse_(s0) << ']'<< std::endl;
    std::cout << "reverse: [" << more::reverse_(s1) << ']'<< std::endl;
    std::cout << "reverse: [" << more::reverse_(s2) << ']'<< std::endl;
    std::cout << "reverse: [" << more::reverse_(s3) << ']'<< std::endl;
 
    return 0;
}
 
