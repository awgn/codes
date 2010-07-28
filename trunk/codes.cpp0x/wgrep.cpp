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
#include <algorithm>
#include <iterator>
#include <functional>
#include <unordered_set>

using namespace std::placeholders;

namespace cpp {

    struct identifier
    {
        template <typename int_type>
        bool operator()(int_type c)
        {
            return std::isalnum(c) || c == '_';    
        }
    };
}

template <typename Pred>
struct basic_token
{
    operator const std::string &() const
    {
        return _M_str;
    }

    template <typename CharT, typename Traits>
    friend inline std::basic_istream<CharT, Traits> & 
    operator>>(std::basic_istream<CharT,Traits> &in, basic_token &rhs)
    {   
        typedef typename std::basic_istream<CharT,Traits>::int_type int_type;
        typedef typename std::basic_istream<CharT,Traits>::ios_base ios_base;

        const int_type eof = Traits::eof();
        typename ios_base::iostate err = ios_base::goodbit; 

        rhs._M_str.erase();

        Pred is_token;

        // skip delimiters:
        int_type c = in.rdbuf()->sgetc();
        while( !Traits::eq_int_type(c, eof) && !is_token(c) )
        {
            c = in.rdbuf()->snextc();
        } 

        if ( Traits::eq_int_type(c, eof) ) {
            err |= ios_base::failbit;
            in.setstate(err);
            return in;
        } 
        
        rhs._M_str.append(1,Traits::to_char_type(c));
        while ( !(c = in.rdbuf()->snextc(), Traits::eq_int_type(c,eof))  && 
                is_token(c) )
        {
            rhs._M_str.append(1,Traits::to_char_type(c));
        }
        
        if ( Traits::eq_int_type(c,eof) ) {
            err |= ios_base::eofbit;
            in.width(0);
            in.setstate(err);            
        }
        return in;
    }

private:
    std::string _M_str;    
};

void grep(const std::string &file_name, const std::unordered_set<std::string> &dict, bool show)
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
        std::vector<std::string> match;

        std::copy_if(std::istream_iterator<basic_token<cpp::identifier>>(ss), 
                     std::istream_iterator<basic_token<cpp::identifier>>(), 
                     std::back_inserter(match),
                     std::bind(&std::unordered_set<std::string>::count, std::ref(dict),_1));
       
        if (match.empty()) {
            continue;
        }
        std::cout << file_name << ':' << c << ':'; 
        if (show) 
            std::copy(match.begin(), match.end(), std::ostream_iterator<std::string>(std::cout, "|"));
        std::cout << line << std::endl;
    }
}

  int
main(int argc, char *argv[])
{
    bool verbose = false;

    std::vector<std::string> args(argv+1, argv+argc);
    auto opt = std::remove(args.begin(), args.end(), std::string("-v"));
    if (opt != args.end()) {
        verbose = true;
        args.erase(opt);
    }

    if (args.size() < 2)
    {
        std::cerr << "usage: wgrep [-v] wordlist file..." << std::endl;
        exit(1);
    }

    // open the worldlist file...
    std::ifstream wordlist(args[0]);
    if (!wordlist) {
        std::cerr << "wgrep: could not open wordlist "<< argv[0] << "!\n";
        exit(1);
    }

    // load the dictionary...
    std::unordered_set<std::string> wordset((std::istream_iterator<std::string>(wordlist)), (std::istream_iterator<std::string>()));

    // run grep
    std::for_each(args.begin()+1, args.end(), std::bind(grep, _1, std::ref(wordset), verbose)); 
    return 0;
}

