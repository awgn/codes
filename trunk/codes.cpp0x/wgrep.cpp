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
#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <iterator>
#include <functional>
#include <unordered_set>

using namespace std::placeholders;

namespace cpp 
{
    extern const char delimiter[] = " '\".,;:[](){}<>";
}

template <const char *Delim>
class basic_token
{
public:
    basic_token()
    : _M_value()
    {}
    
    ~basic_token()
    {}

    operator const std::string &() const
    {
        return _M_value;
    }

    template <typename CharT, typename Traits>
    friend inline std::basic_istream<CharT, Traits> & 
    operator>>(std::basic_istream<CharT,Traits> &__in, basic_token &rhs)
    {
        char c;            
        rhs._M_value.clear();

        __in >> std::noskipws;
        while( __in >> c && strchr(Delim,c) )
        {
            // skip delimiter
        }
        if (__in) {
            rhs._M_value.append(1,c);
            while ( (__in >> c) && !strchr(Delim,c) )
            {
                rhs._M_value.append(1,c);
            }
        }
        return __in;
    }

private:
    std::string _M_value;    
};


void grep(const std::string &file_name, const std::unordered_set<std::string> &dict)
{
    std::ifstream file(file_name);
    if (!file)
    {
        std::cerr << "Could not open " << file_name << " for reading (skipped)" << std::endl;
        return;
    }

    std::string line;
    for(int c = 1; std::getline(file, line); c++)
    {
        std::istringstream ss(line);

        // load token from the istreamstream
        std::vector<std::string> vec( (std::istream_iterator<basic_token<cpp::delimiter>>(ss)),
                                      (std::istream_iterator<basic_token<cpp::delimiter>>() ));

        // find the first occurrence in the wordlist
        if ( std::find_if(vec.begin(), vec.end(), std::bind(&std::unordered_set<std::string>::count, std::ref(dict), _1) ) != vec.end() )
        {
            std::cout << file_name << ':' << c << ':' << line << std::endl;
        }
    }
}

  int
main(int argc, char *argv[])
{
    if (argc<3)
    {
        std::cerr << "usage: wgrep wordlist file..." << std::endl;
        exit(1);
    }

    std::unordered_set<std::string> wordset;
    {
        // open the worldlist file...
        std::ifstream wordlist(argv[1]);
        if (!wordlist) {
            std::cerr << "wgrep: could not open wordlist "<< argv[1] << "!\n";
            exit(2);
        }

        // load the dictionary...
        wordset.insert(std::istream_iterator<std::string>(wordlist), std::istream_iterator<std::string>());
    }

    std::for_each(argv+2, argv+argc, std::bind(grep, _1, std::ref(wordset))); 
    return 0;
}

