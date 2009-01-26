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
#include <fstream>
#include <vector>
#include <iterator>

int
main(int argc, char *argv[])
{
    std::ifstream file;
    file.open("FILENAME", std::ios::in | std::ios::binary);
    if (!file) {
        std::cout << "error: couldn't open file.\n";
        return 1;
    }
    std::vector<char> mfile;
    std::copy(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>(),
              std::back_inserter(mfile));

    return 0;
}
 
