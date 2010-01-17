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
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <vector>

#include <yacpp.hh>

int
main(int argc, char *argv[])
{
    std::ifstream in;
    
    {
        std::cout << "----------- code lines ----------" << std::endl;
        in.open(argv[1]); 
        if (!in)
            throw std::runtime_error("ifstream");
    
        more::yacpp::line_iterator it(in);
        while ( it != more::yacpp::line_iterator() ) {
            std::cout << "[" << *it++ << "]" << std::endl;
        }
    
        in.close();
    }

    {
        std::cout << "----------- cpp lines (only) ----------" << std::endl;
        in.open(argv[1]); 
        if (!in)
            throw std::runtime_error("ifstream");

        more::yacpp::cpp_iterator ct(in);
        while ( ct != more::yacpp::cpp_iterator() ) {
            std::cout << "[" << *ct++ << "]" << std::endl;
        }

        in.close();
    }

    {
        std::cout << "----------- comment lines ----------" << std::endl;
        in.open(argv[1]); 
        if (!in)
            throw std::runtime_error("ifstream");

        more::yacpp::comment_iterator ct(in);
        while ( ct != more::yacpp::comment_iterator() ) {
            std::cout << "[" << *ct++ << "]" << std::endl;
        }

        in.close();
    }

    // test: iterator and STL algorithm
    std::cout << "----- iterator (preproc) -----" << std::endl;

    in.open(argv[1]);
    std::copy(more::yacpp::cpp_iterator(in),
              more::yacpp::cpp_iterator(),
              std::ostream_iterator<std::string>(std::cout, "\n"));

    in.close();

    return 0;
}
 
