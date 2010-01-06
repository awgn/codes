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
#include <sstream>
#include <vector>

int
main(int argc, char *argv[])
{
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
        std::cout << "trim: [" << more::trim_copy(s0) << ']' << std::endl;
        std::cout << "trim: [" << more::trim_copy(s1) << ']' << std::endl;
        std::cout << "trim: [" << more::trim_copy(s2) << ']' << std::endl;
        std::cout << "trim: [" << more::trim_copy(s3) << ']' << std::endl;

        std::cout << "trim! [" << more::trim(s0) << ']' << std::endl;
        std::cout << "trim! [" << more::trim(s1) << ']' << std::endl;
        std::cout << "trim! [" << more::trim(s2) << ']' << std::endl;
        std::cout << "trim! [" << more::trim(s3) << ']' << std::endl;
    }

    {
        std::string s0("aB Ab");
        std::string s1("   aB Ab");
        std::string s2("aB Ab  ");
        std::string s3("   aB Ab  ");

        std::cout << std::endl;
        std::cout << "left_trim: [" << more::left_trim_copy(s0) << ']' << std::endl;
        std::cout << "left_trim: [" << more::left_trim_copy(s1) << ']' << std::endl;
        std::cout << "left_trim: [" << more::left_trim_copy(s2) << ']' << std::endl;
        std::cout << "left_trim: [" << more::left_trim_copy(s3) << ']' << std::endl;

        std::cout << "left_trim! [" << more::left_trim(s0) << ']' << std::endl;
        std::cout << "left_trim! [" << more::left_trim(s1) << ']' << std::endl;
        std::cout << "left_trim! [" << more::left_trim(s2) << ']' << std::endl;
        std::cout << "left_trim! [" << more::left_trim(s3) << ']' << std::endl;
    }

    {
        std::string s0("aB Ab");
        std::string s1("   aB Ab");
        std::string s2("aB Ab  ");
        std::string s3("   aB Ab  ");

        std::cout << std::endl;
        std::cout << "right_trim: [" << more::right_trim_copy(s0) << ']' << std::endl;
        std::cout << "right_trim: [" << more::right_trim_copy(s1) << ']' << std::endl;
        std::cout << "right_trim: [" << more::right_trim_copy(s2) << ']' << std::endl;
        std::cout << "right_trim: [" << more::right_trim_copy(s3) << ']' << std::endl;

        std::cout << "right_trim! [" << more::right_trim(s0) << ']' << std::endl;
        std::cout << "right_trim! [" << more::right_trim(s1) << ']' << std::endl;
        std::cout << "right_trim! [" << more::right_trim(s2) << ']' << std::endl;
        std::cout << "right_trim! [" << more::right_trim(s3) << ']' << std::endl;
    }

    {
        std::string s0("aB Ab");
        std::string s1("   aB Ab");
        std::string s2("aB Ab  ");
        std::string s3("   aB Ab  ");


        std::cout << std::endl;
        std::cout << "swapcase: [" << more::swapcase_copy(s0) << ']' << std::endl;
        std::cout << "swapcase: [" << more::swapcase_copy(s1) << ']' << std::endl;
        std::cout << "swapcase: [" << more::swapcase_copy(s2) << ']' << std::endl;
        std::cout << "swapcase: [" << more::swapcase_copy(s3) << ']' << std::endl;

        std::cout << "swapcase! [" << more::swapcase(s0) <<']' <<  std::endl; // in-place version...
        std::cout << "swapcase! [" << more::swapcase(s1) <<']' <<  std::endl;
        std::cout << "swapcase! [" << more::swapcase(s2) <<']' <<  std::endl;
        std::cout << "swapcase! [" << more::swapcase(s3) <<']' <<  std::endl;

        std::cout << std::endl;
        std::cout << "upcase: [" << more::upcase_copy(s0) << ']' << std::endl;
        std::cout << "upcase: [" << more::upcase_copy(s1) << ']' << std::endl;
        std::cout << "upcase: [" << more::upcase_copy(s2) << ']' << std::endl;
        std::cout << "upcase: [" << more::upcase_copy(s3) << ']' << std::endl;

        std::cout << "upcase! [" << more::upcase(s0) <<']' <<  std::endl;  // in-place version...
        std::cout << "upcase! [" << more::upcase(s1) <<']' <<  std::endl;
        std::cout << "upcase! [" << more::upcase(s2) <<']' <<  std::endl;
        std::cout << "upcase! [" << more::upcase(s3) <<']' <<  std::endl;

        std::cout << std::endl;
        std::cout << "downcase: [" << more::downcase_copy(s0) << ']'<< std::endl;
        std::cout << "downcase: [" << more::downcase_copy(s1) << ']'<< std::endl;
        std::cout << "downcase: [" << more::downcase_copy(s2) << ']'<< std::endl;
        std::cout << "downcase: [" << more::downcase_copy(s3) << ']'<< std::endl;

        std::cout << "downcase! [" << more::downcase(s0) <<']'<<  std::endl; // in-place version...
        std::cout << "downcase! [" << more::downcase(s1) <<']'<<  std::endl;
        std::cout << "downcase! [" << more::downcase(s2) <<']'<<  std::endl;
        std::cout << "downcase! [" << more::downcase(s3) <<']'<<  std::endl;

        std::cout << std::endl;
        std::cout << "casecmp: " << std::boolalpha << more::casecmp("aB","Ab") << std::endl;

        std::cout << std::endl;
        std::cout << "capitalize: [" << more::capitalize_copy(s0) << ']'<< std::endl;
        std::cout << "capitalize: [" << more::capitalize_copy(s1) << ']'<< std::endl;
        std::cout << "capitalize: [" << more::capitalize_copy(s2) << ']'<< std::endl;
        std::cout << "capitalize: [" << more::capitalize_copy(s3) << ']'<< std::endl;

        std::cout << "capitalize! [" << more::capitalize(s0) << ']'<< std::endl; // in-place version... 
        std::cout << "capitalize! [" << more::capitalize(s1) << ']'<< std::endl;
        std::cout << "capitalize! [" << more::capitalize(s2) << ']'<< std::endl;
        std::cout << "capitalize! [" << more::capitalize(s3) << ']'<< std::endl;

        std::cout << std::endl;
        std::cout << "reverse: [" << more::reverse_copy(s0) << ']'<< std::endl;
        std::cout << "reverse: [" << more::reverse_copy(s1) << ']'<< std::endl;
        std::cout << "reverse: [" << more::reverse_copy(s2) << ']'<< std::endl;
        std::cout << "reverse: [" << more::reverse_copy(s3) << ']'<< std::endl;

        std::cout << "reverse! [" << more::reverse(s0) << ']'<< std::endl; // in-place version...
        std::cout << "reverse! [" << more::reverse(s1) << ']'<< std::endl;
        std::cout << "reverse! [" << more::reverse(s2) << ']'<< std::endl;
        std::cout << "reverse! [" << more::reverse(s3) << ']'<< std::endl;
    }

    // replace test
    //

    {
        std::string abc("1 2 3");
   
        std::cout << std::endl;
        std::cout << "orig  [" << abc << "]" << std::endl;
        std::cout << "repl: [" << more::repl(abc, " ", "  ")  << "]" << std::endl;
        more::repl(abc, " " , "__");
        std::cout << "repl_ [" << abc << "]" << std::endl;
    }    

    // split test
    //

    {
        std::string abc("1 2, 3");
        std::vector<std::string> vec;

        std::cout << std::endl;
        std::cout << "split: [";
       
        more::split(abc, std::back_inserter(vec), " ,"); 

        std::copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(std::cout, " "));
        std::cout << "]" << std::endl;
    }    


    // join test 
    //
    {
        std::vector<std::string> abc;
        abc.push_back("1");
        abc.push_back("2");
        abc.push_back("3");
        
        std::cout << std::endl;
        std::cout << "join: [" << more::join(abc.begin(), abc.end()) << "]" << std::endl;
        std::cout << "join: [" << more::join(abc.begin(), abc.end(), "_") << "]" << std::endl;
    }    

    // extending getline...
    //
    {
        std::string str("one:two: three \nfour");
        std::istringstream sstr(str);

        std::cout << std::endl;

        std::string l;
        while ( more::getline(sstr, l, std::string("\n: ") ) ) 
        {
            std::cout << "getline: [" << l << "]" << std::endl;
        }
    }

    // extending getline with escape support for separator...
    //
    {
        std::string str("one\\:two: \\three\\\\ \nfour \\  ");
        std::istringstream sstr(str);

        // std::ifstream sstr("test.cc");

        std::cout << std::endl;

        std::string l;
        while ( more::getline_esc(sstr, l, std::string("\n: ") ) ) 
        {
            std::cout << "getline_esc: [" << l << "]" << std::endl;
        }
    }


    // in-place versions return const reference to avoid misuse...
    // example:

    // std::string str(" one two ");
    // more::upcase(more::trim(str));              // not allowed!
    // more::upcase_copy(more::trim(str));         // ok! making an upcase copy of the trimmed str.
    // more::upcase(more::trim_copy(str));         // not allowed: upcase() on temporary!
    // more::upcase_copy(more::trim_copy(str));    // ok! 
    
    return 0;
}
 
