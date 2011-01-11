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
#include <sstream>

#include <iomanip.hpp>
#include <yats.hpp>
using namespace yats;

Context(more_iomanip_test)
{
    Test(spaces)
    {
        std::ostringstream out;
        out << more::spaces(4);
        Assert(out.str(), is_equal_to(std::string("    ")));   
    }

    Test(string_token)
    {
        std::istringstream in("this line is ignored\none:two: three \nfour");
        in >> more::ignore_line;  // <- ignore_line

        std::vector<std::string> vec = { "one", "two", "three", "four" };
        std::vector<std::string> out;
        
        more::string_token tok("\n: "); // <- string_token

        while(in >> tok)
            out.push_back(tok.str());

        Assert((std::equal(out.begin(), out.end(), vec.begin())), is_true());
    }

    Test(wstring_token)
    {
        std::wistringstream in(L"this line is ignored\none:two: three \nfour");
        in >> more::ignore_line;  // <- ignore_line

        std::vector<std::wstring> vec = { L"one", L"two", L"three", L"four" };
        std::vector<std::wstring> out;
        
        more::wstring_token tok(L"\n: "); // <- string_token

        while(in >> tok)
            out.push_back(tok.str());

        Assert((std::equal(out.begin(), out.end(), vec.begin())), is_true());
    }

    Test(string_line)
    {
        std::istringstream in("this line is ignored\none:two: three \nfour\nescaped\\\nnewline");
        in >> more::ignore_line;  // <- ignore_line

        std::vector<std::string> vec = { "one:two: three ", "four", "escaped\nnewline" };
        std::vector<std::string> out;

        more::string_line line; // <- string_token

        while(in >> line)
            out.push_back(line.str());

        Assert((std::equal(out.begin(), out.end(), vec.begin())), is_true());
    }

    Test(wstring_line)
    {
        std::wistringstream in(L"this line is ignored\none:two: three \nfour\nescaped\\\nnewline");
        in >> more::ignore_line;  // <- ignore_line

        std::vector<std::wstring> vec = { L"one:two: three ", L"four", L"escaped\nnewline" };
        std::vector<std::wstring> out;

        more::wstring_line line; // <- string_token

        while(in >> line)
            out.push_back(line.str());

        Assert((std::equal(out.begin(), out.end(), vec.begin())), is_true());
    }
}
 
int
main(int argc, char *argv[])
{
    return yats::run();
}
