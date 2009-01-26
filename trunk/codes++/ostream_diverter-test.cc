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
#include <ostream_diverter.h>

class Test : protected more::osd<Test>
{
    public:
    Test() 
    {
        cout << "hello world\n";
    }

    using more::osd<Test>::cout_divert; 
    using more::osd<Test>::cerr_divert; 
    using more::osd<Test>::clog_divert; 
};

int main()
{
    Test::cout_divert(std::cerr);
    Test x;
}
