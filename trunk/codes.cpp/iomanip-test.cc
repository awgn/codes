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

#include <iomanip.hh>

int
main(int, char *[])
{
    // string...
    {
        std::string str("this line is ignored\none:two: three \nfour");
        std::istringstream sstr(str);

        sstr >> more::ignore_line;  // <- ignore_line

        more::string_token tok("\n: "); // <- string_token
        while (sstr >> tok)
        {
            std::cout << more::spaces(4) << "string_token: [" << tok.str() << "]" << std::endl;  // <- spaces
        }
    }

    // wstring...
    {
        std::wstring str(L"this line is ignored\none:two: three \nfour");
        std::wistringstream sstr(str);

        sstr >> more::ignore_line;  // <- ignore_line

        more::wstring_token tok(L"\n: "); // <- string_token
        while (sstr >> tok)
        {
            std::wcout << more::spaces(3) << L"string_wtoken: [" << tok.str() << L"]" << std::endl;  // <- spaces
        }
    }

    // string...
    {
        std::string str("this line is ignored\none:two: three \nfour\nescaped\\\nnewline");
        std::istringstream sstr(str);

        sstr >> more::ignore_line;  // <- ignore_line

        more::string_line line; // <- token_line
        while (sstr >> line)
        {
            std::cout << more::spaces(5) << "string_line: [" << line.str() << "]" << std::endl;  // <- spaces
        }
    }

    // wstring...
    {
        std::wstring str(L"this line is ignored\none:two: three \nfour\nescaped\\\nnewline");
        std::wistringstream sstr(str);

        sstr >> more::ignore_line;  // <- ignore_line

        more::wstring_line line; // <- token_line
        while (sstr >> line)
        {
            std::wcout << more::spaces(4) << L"wstring_line: [" << line.str() << L"]" << std::endl;  // <- spaces
        }
    }


    return 0;
}
 
