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
#include <iomanip>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <tuple>
#include <stack>
#include <memory>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <functional>
#include <stdexcept>

#include <token.hpp>   // more!

namespace cpp 
{
    struct identifier
    {
        template <typename int_type>
        bool operator()(int_type c)
        {
            return std::isalnum(c) || c == '_';    
        }
    };
}
 
enum dir_class { 
    _define, _undef, _include, 
    _if, _if_else, _else, 
    _elif, _elif_else, _endif, 
    _ifdef, _ifdef_else, 
    _ifndef, _ifndef_else, 
    _reserved,
    _sizeof_dir_class }; 

const std::vector<std::string> static_dirname =
{
    "define", "undef", "include", 
    "if", "if_else", "else", 
    "elif", "elif_else", "endif", 
    "ifdef", "ifdef_else", 
    "ifndef", "ifndef_else", 
    "reserved", "" 
};

typedef std::tuple< dir_class,   /* directive class */
                    bool,        /* else statement */ 
                    std::string  /* id */
                    > dir_type;

typedef std::tuple< std::string, /* source code */
                    int          /* line */ 
                    > info_type;

typedef std::map<dir_type,std::list<info_type>> map_type;

typedef std::map<std::string, dir_class> dirmap_type; 

const dirmap_type static_dirmap = 
{{ "include"    , _include     },
 { "define"     , _define      },
 { "undef"      , _undef       }, 
 { "if"         , _if          },
 { "if_else"    , _if_else     },
 { "else"       , _else        },
 { "endif"      , _endif       },
 { "elif"       , _elif        },
 { "elif_else"  , _elif_else   },
 { "ifdef"      , _ifdef       },
 { "ifdef_else" , _ifdef_else  },
 { "ifndef"     , _ifndef      },
 { "ifndef_else", _ifndef_else }};

const dirmap_type static_queries = 
{{ "include"    , _include     },
 { "define"     , _define      },
 { "undef"      , _undef       }, 
 { "if"         , _if          },
 { "if_else"    , _if_else     },
 { "elif"       , _elif        },
 { "elif_else"  , _elif_else   },
 { "ifdef"      , _ifdef       },
 { "ifdef_else" , _ifdef_else  },
 { "ifndef"     , _ifndef      },
 { "all"        , _reserved    },
 { "ifndef_else", _ifndef_else }};
 
int
get_class_type(const dir_type &d)
{
    return std::get<0>(d) + (std::get<1>(d) ? 1 : 0);
}

std::vector<std::pair<dir_type, info_type>>
parse_source(const std::string &name)
{
    std::vector<std::pair<dir_type,info_type>> ret;

    std::ifstream file(name);
    if(!file)
        std::runtime_error(std::string("ifstream: could not open ").append(name));

    std::string line;
    std::stack<std::pair<dir_type,info_type>> tmp;

    for(int c = 1; std::getline(file, line); c++)
    {
        std::istringstream ss(line); std::string token;
        ss >> token;

        if (token[0] != '#')   // skip non-preprocessor directive
            continue; 

        token.erase(0,1);   // erase '#' char
        auto it = static_dirmap.find(token);
        if (it != static_dirmap.end())
        {
            more::basic_token<cpp::identifier> id; ss >> id;

            switch(it->second)
            {
            case _include:
            case _define:
            case _undef:
                ret.push_back(std::make_pair(std::make_tuple(it->second, false, static_cast<std::string>(id)),
                                             std::make_tuple(name,c)));
                break;
            case _if:
                tmp.push(std::make_pair(std::make_tuple(it->second, false, ss.str()),
                                        std::make_tuple(name,c)));
                break;
            case _ifdef:
            case _ifndef:
                tmp.push(std::make_pair(std::make_tuple(it->second, false, static_cast<std::string>(id)),
                                        std::make_tuple(name,c)));
                break;
            case _else:
                if (tmp.empty()) {
                    std::ostringstream error;
                    error << "parse error at " << name << ':' << c; 
                    throw std::runtime_error(error.str()); 
                }
                std::get<1>(tmp.top().first) = true;
                break;
            case _elif:
                if (tmp.empty()) {
                    std::ostringstream error;
                    error << "parse error at " << name << ':' << c; 
                    throw std::runtime_error(error.str()); 
                }
                std::get<1>(tmp.top().first) = true;
                tmp.push(std::make_pair(std::make_tuple(it->second, false, ss.str()),
                                        std::make_tuple(name,c)));
                break;
            case _endif:
                if (tmp.empty()) {
                    std::ostringstream error;
                    error << "parse error at " << name << ':' << c; 
                    throw std::runtime_error(error.str()); 
                }
                ret.push_back(tmp.top());
                tmp.pop();
                break;
            }
        }
    }

    return ret;
}


struct dump_query {

    dump_query(bool verbose, bool gcc)
    : _M_verbose(verbose), _M_gcc(gcc)
    {}

    void operator()(const map_type::value_type &elem) const
    {    
        const dir_type &d = elem.first;

        if (_M_gcc) 
        {
            std::cout << "-D" << std::get<2>(d) << std::endl;
            return;
        } 
        
        std::cout << std::left << std::setw(16) << static_dirname[get_class_type(d)] << ": " << std::get<2>(d);
        if (!_M_verbose) {
            std::cout << std::endl;
            return;
        }

        std::cout << "\t( ";
        auto lit = elem.second.cbegin();
        auto lit_end = elem.second.cend();
        for(;lit != lit_end; ++lit)
        {
            std::cout << std::get<0>(*lit) << ':' << std::get<1>(*lit) << ' ';
        }
        std::cout << ")\n";
    }
    
    bool _M_verbose;
    bool _M_gcc;
};


struct query_predicate
{
    query_predicate(const std::vector<bool> &plus, const std::vector<bool> &minus)
    : _M_plus(plus), _M_minus(minus)
    {}

    bool operator()(const map_type::value_type &elem) const
    {
        const dir_type &d = elem.first;
        return _M_plus[get_class_type(d)] && !_M_minus[get_class_type(d)];
    }

    const std::vector<bool> &_M_plus; 
    const std::vector<bool> &_M_minus; 
};


struct map_pusher
{
    map_pusher(map_type &m)
    : _M_map(m)
    {}

    void operator()(const std::pair<dir_type, info_type> &p) const
    {
        _M_map[p.first].push_back(p.second);
    }

    map_type &_M_map;
};

void usage()
{
    std::cerr << "usage: ppan [opts] sources...\n";
    std::cerr << 
    "queries:   \n"
    "  -+include\n"    
    "  -+define\n"     
    "  -+undef\n"      
    "  -+if\n"         
    "  -+if_else\n"    
    "  -+elif\n"       
    "  -+elif_else\n"  
    "  -+ifdef\n"      
    "  -+ifdef_else\n" 
    "  -+ifndef\n"     
    "  -+ifndef_else\n"
    "  -+all\n"
    "options:\n"
    "  -g,--gcc        adapt output as gcc/g++\n"
    "  -v,--verbose\n"
    "  -h,--help" << std::endl;
    exit(1);
}

  int
main(int argc, char *argv[])
{
    // simple options:
    bool verbose = false;
    bool gcc = false;

    std::vector<std::string> args(argv+1, argv+argc);
    if (args.size() < 1)
        usage();

    std::vector<bool> plus_opt(_sizeof_dir_class);
    std::vector<bool> minus_opt(_sizeof_dir_class);

    auto opt = args.begin();
    auto opt_end = args.end();

    for(; opt != opt_end; ++opt)
    {   
        int v = ((opt->at(0) == '-') * -1 +
                 (opt->at(0) == '+') *  1);

        if (v == 0)
            break; 

        std::string token(opt->data() + 1);  // skip +/-
        auto it = static_queries.find(token);
        if (it != static_queries.end())
        {
            if (v > 0) {
                if (it->second == _reserved)
                    plus_opt.assign(plus_opt.size(), true);
                plus_opt[it->second] = true;
            }
            else {
                if (it->second == _reserved)
                    minus_opt.assign(minus_opt.size(), true);
                minus_opt[it->second] = true;
            }
        }
        else { // other options:
            if (*opt == "-v" || *opt == "--verbose")
            {
                verbose = true; continue;
            }             
            if (*opt == "-g" || *opt == "--gcc")
            {
                minus_opt[_include] = true;
                gcc = true; continue;
            }
            if (*opt == "-h" || *opt == "--help")
            {
                usage();
            }
            std::cerr << *opt << ": unknow option!" << std::endl;
            exit(1);
        } 
    }

    if ( opt == opt_end )
        usage();

    map_type macro;

    auto it = args.begin();
    auto it_end = args.end();
    for(; it != it_end; ++it)
    {
        auto v = parse_source(*it);
        std::for_each(v.begin(), v.end(), map_pusher(macro));
    }

    // query select:

    map_type result;
    std::copy_if(macro.begin(), macro.end(),
                 std::inserter(result, result.begin()), 
                 query_predicate(plus_opt, minus_opt));
 
    std::for_each(result.begin(), result.end(), dump_query(verbose,gcc));
    return 0;
}

