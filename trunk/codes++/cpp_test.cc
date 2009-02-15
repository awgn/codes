/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <tr1/functional>
#include <iostream>
#include <algorithm>
#include <string>
#include <list>
#include <iterator>

#include <cstring>
#include <cstdlib>
#include <cassert>

//////////////////////// more library dependency

#include <kv_parser.hh>
#include <format.hh>
#include <exec.hh>
#include <colorful.hh>

//////////////////////// glibc headers

#include <sys/time.h>
#include <getopt.h>
#include <errno.h>

#define member_reader(type,value) \
    const type & \
    value() const \
    { return _M_ ## value; }

#define member_writer(cl, type, value) \
    cl & \
    value (const type &v) \
    { _M_ ## value = v; return *this; }

#define member_push_back(cl, type, value) \
    cl & \
    value (const std::string &v) \
    {   \
        std::istringstream in(v);   \
        std::copy(std::istream_iterator<type>(in), std::istream_iterator<type>(), \
                  std::back_inserter(_M_ ## value)); \
        return *this; \
    }

extern char *__progname;
const char usage[]=
     "%s [options] source.cpp:\n"
      "   -t target           cpp test file\n"
      "   -l library          link library\n"
      "     -pthread          link pthread library\n"
      "   -L library/path     add library directory\n" 
      "   -I include/path     add include directory\n\n"
      "   -D macro            define a macro\n"
      "   -c conf             config file\n"
      "   -h                  print this help.\n";

const std::string  COMPILE_ERROR("COMPILE_ERROR_");
const std::string  WARNING_ERROR("WARNING_ERROR_");

using std::tr1::reference_wrapper;
using namespace std::tr1::placeholders;

typedef more::colorful< TYPELIST(more::ecma::bold, more::ecma::fg_red)>  RED;
typedef more::colorful< TYPELIST(more::ecma::bold, more::ecma::fg_blue)> BLUE;
typedef more::colorful< TYPELIST(more::ecma::reset)> RESET;
typedef more::colorful< TYPELIST(more::ecma::bold)> BOLD;

class testcpp
{   
    std::string             _M_compiler;
    std::list<std::string>  _M_opt;
    std::list<std::string>  _M_macro;
    std::list<std::string>  _M_source;
    std::list<std::string>  _M_library;
    std::list<std::string>  _M_include;

    int                     _M_run_status;
    int                     _M_run_expect_status;
	int						_M_compile_status;
    int                     _M_compile_expect_status;

    bool                    _M_verbose_compile;
    bool                    _M_verbose_run;

    int				        _M_id;
    static int				_S_id;

	std::string bin() const
	{ return more::format("testbin_%1") % _M_id; }

public:
    testcpp()
    : _M_compiler(),
      _M_opt(),
      _M_macro(),
      _M_source(),
      _M_library(),
      _M_run_status(-1),
      _M_run_expect_status(0),
	  _M_compile_status(-1),
      _M_compile_expect_status(0),
      _M_verbose_compile(true),
      _M_verbose_run(true),
      _M_id(_S_id++)
	{}

    ~testcpp()
    {
        unlink(this->bin().c_str());
    }

    member_reader(std::string,compiler);
    member_writer(testcpp, std::string,compiler);

    member_reader(std::list<std::string>,opt);
    member_push_back(testcpp, std::string,opt);

    member_reader(std::list<std::string>,macro);
    member_push_back(testcpp, std::string,macro);

    member_reader(std::list<std::string>,source);
    member_push_back(testcpp, std::string,source);

    member_reader(std::list<std::string>,library);
    member_push_back(testcpp, std::string, library);

    member_reader(std::list<std::string>,include);
    member_push_back(testcpp, std::string, include);

    member_reader(bool, verbose_compile);
    member_writer(testcpp, bool, verbose_compile);

    member_reader(bool, verbose_run);
    member_writer(testcpp, bool, verbose_run);

    member_reader(int, compile_status);
    member_reader(int, run_status);

    /////////////////////////////////////////////// typesafe enum 

    template <int v>
    struct test_expectation
    { enum { value = v }; };

    struct success : public test_expectation<0> 
    {};
    
    struct failure : public test_expectation<1> 
    {};

    /////////////////////////////////////////////// compile

    template <int v>
    void compile_expectation(test_expectation<v>)
    { _M_compile_expect_status = v; }

    void compile()
    {
        more::exec cc(_M_compiler, ::execvp );

        std::for_each(_M_opt.begin(), _M_opt.end(),         std::tr1::bind(&more::exec::arg, std::tr1::ref(cc), _1 ));
        std::for_each(_M_macro.begin(), _M_macro.end(),     std::tr1::bind(&more::exec::arg, std::tr1::ref(cc), _1 ));
        std::for_each(_M_include.begin(), _M_include.end(), std::tr1::bind(&more::exec::arg, std::tr1::ref(cc), _1 ));
        std::for_each(_M_source.begin(), _M_source.end(),   std::tr1::bind(&more::exec::arg, std::tr1::ref(cc), _1 )); 

        cc.arg("-o");
        cc.arg(this->bin());

        std::for_each(_M_library.begin(), _M_library.end(),std::tr1::bind(&more::exec::arg, std::tr1::ref(cc), _1)); 

        if ( _M_compile_expect_status ) {
            std::cout << RED() << "compile: " << RESET() << cc.cmd() << std::endl;
        }
        else {
            std::cout << BOLD() << "compile: " << RESET() << cc.cmd() << std::endl;
        }

        if ( _M_verbose_compile ) {

            if ( ! cc() ) {
                std::cout << __FUNCTION__ << ": compile error: " << strerror(errno) << std::endl;
                return;
            }

        } else {

            int n;
            if ( ! cc( more::exec::redirect_fd<2>(n) ) ) {
                std::cout << __FUNCTION__ << ": compile error: " << strerror(errno) << std::endl;
                return;
            }

        }

        cc.wait();
        if ( cc.is_exited() ) 
            _M_compile_status = cc.exit_status();
    }

    /////////////////////////////////////////////// run 

    template <int v>
    void run_expectation(test_expectation<v>)
    { _M_run_expect_status = v; }

	bool run()
	{
        if ( _M_compile_expect_status != 0 )
            return false;

        more::exec target("./" + this->bin(), ::execvp );
        std::cout << BOLD() << "running " << RESET() << "./" << this->bin();

        timeval a, b;

        gettimeofday(&a, NULL);
        if (!target()) {
            std::cout << std::endl << __FUNCTION__<< ": couldn't run " << target.cmd() 
                      << ": " << strerror(errno) << std::endl;
        }
        target.wait();
        gettimeofday(&b, NULL);

        int usec = b.tv_usec - a.tv_usec;
        int sec = b.tv_sec - a.tv_sec;

        if (usec < 0) {
            sec  -= 1;
            usec += 1000000;
        }

        std::cout << " time: { sec=" << sec << " usec=" << usec << " } " << std::endl;

        if ( target.is_exited() ) 
		    _M_run_status = target.exit_status();

        return true;    
	}
	
    /////////////////////////////////////////////// posteriori checks 

    bool compile_succeeded() const
    {
        return ! ( !!_M_compile_status ^ !!_M_compile_expect_status ); 
    }
    bool run_succeeded() const
    {
        return ! ( !!_M_run_status ^ !!_M_run_expect_status ); 
    }

};

int testcpp::_S_id = 0;

///////////////////////// testcpprc parser 

TYPEMAP_KEY(compiler);
TYPEMAP_KEY(warning_opt);
TYPEMAP_KEY(optimization_opt);

typedef TYPEMAP(compiler,         std::vector<std::string>,
                warning_opt,      std::vector<std::string>,
                optimization_opt, std::vector<std::string>) rcfile_type;

struct cpptestrc : public more::kv::parser<rcfile_type> {};

cpptestrc config;

///////////////////////// global objects 

std::vector<testcpp>     cpptests;
std::vector<std::string> libraries;
std::vector<std::string> includes;
std::vector<std::string> macros;

///////////////////////// free functions

bool
parse_ifdef(const std::string &source, std::vector<std::string> &ret, const std::string &match = std::string())
{
    std::vector<std::string> commit;

    std::ifstream src(source.c_str());
    std::string line;

    while( getline(src,line) ) {
        std::istringstream in(line);
        std::string directive;
        in >> directive;

        if ( directive == "#ifdef" ) {
            std::string key;
            in >> key;
            if (!match.empty() && key.compare(0,match.size(),match) )
                continue;

            commit.push_back(key);
        }
    }

    std::swap(commit,ret);
    return true;
}

bool load_cpptests(const std::string &target)
{
    std::vector<std::string>::const_iterator comp = config.get<compiler>().begin();
    std::vector<std::string>::const_iterator warn = config.get<warning_opt>().begin();
    std::vector<std::string>::const_iterator opt  = config.get<optimization_opt>().begin();

    ///////////////////// simple compile test

    std::cout << "\nloading compile tests: ";

    // for each compiler

    for( ; comp != config.get<compiler>().end(); ++comp)
    {
        std::cout << '+';

        // for each warning option

        warn = config.get<warning_opt>().begin();
        for(; warn != config.get<warning_opt>().end(); ++warn)
        {
            testcpp x;
            x.compiler(*comp).source(target);
           
            std::vector<std::string> copt;
            std::vector<std::string>::const_iterator it = config.get<warning_opt>().begin();
            do {
                copt.push_back(*it);
            } while ( it++ != warn);

            std::for_each(copt.begin(), copt.end(), 
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::opt), std::tr1::ref(x), _1));  

            // load libraries
            std::for_each(libraries.begin(), libraries.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::library), std::tr1::ref(x), _1));  

            // load includes
            std::for_each(includes.begin(), includes.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::include), std::tr1::ref(x), _1));  

            // load macros 
            std::for_each(macros.begin(), macros.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::macro), std::tr1::ref(x), _1));  


            x.compile_expectation( testcpp::success() );
            x.run_expectation( testcpp::success() );

            std::cout << '.';
            cpptests.push_back(x);
        }
    } 

    std::cout << BOLD() << " [done]" << RESET() << std::endl;

    ///////////////////// compile errors test

    comp = config.get<compiler>().begin();
    warn = config.get<warning_opt>().begin();
    opt  = config.get<optimization_opt>().begin();

    std::cout << "loading compile errors tests: ";

    std::vector<std::string> ifdef;
    parse_ifdef(target, ifdef, COMPILE_ERROR);
   
    // for each compiler

    for( ; comp != config.get<compiler>().end(); ++comp)
    {
        std::cout << '+';

        // for each compile error option

        std::vector<std::string>::const_iterator it = ifdef.begin();
        for(; it != ifdef.end(); ++it) 
        {
            testcpp x;
            x.compiler(*comp).source(target);
         
            std::string errmacro("-D");
            errmacro.append(*it);

            // add the macro
            x.opt(errmacro);

            // load libraries
            std::for_each(libraries.begin(), libraries.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::library), std::tr1::ref(x), _1));  

            // load includes
            std::for_each(includes.begin(), includes.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::include), std::tr1::ref(x), _1));  

            // load macros 
            std::for_each(macros.begin(), macros.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::macro), std::tr1::ref(x), _1));  

            x.compile_expectation( testcpp::failure() );
            x.verbose_compile(false);

            std::cout << '.';
            cpptests.push_back(x);
        }
    } 

    std::cout << BOLD() << " [done]" << RESET() << std::endl;

    ///////////////////// runtime test

    comp = config.get<compiler>().begin();
    warn = config.get<warning_opt>().begin();
    opt  = config.get<optimization_opt>().begin();

    // for each compiler

    std::cout << "loading runtime tests: ";

    for( ; comp != config.get<compiler>().end(); ++comp)
    {
        std::cout << '+';

        // for each optimization option
        opt = config.get<optimization_opt>().begin();
        for(; opt != config.get<optimization_opt>().end(); ++opt)
        {
            testcpp x;
            x.compiler(*comp).source(target);

            x.opt(*opt);

            // load libraries
            std::for_each(libraries.begin(), libraries.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::library), std::tr1::ref(x), _1));  

            // load includes
            std::for_each(includes.begin(), includes.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::include), std::tr1::ref(x), _1));  

            // load macros 
            std::for_each(macros.begin(), macros.end(),
                          std::tr1::bind( static_cast<testcpp &(testcpp::*)(const std::string &)>(&testcpp::macro), std::tr1::ref(x), _1));  

            x.compile_expectation( testcpp::success() );
            x.run_expectation( testcpp::success() );

            std::cout << '.';
            cpptests.push_back(x);
        }
    } 
    
    std::cout << BOLD() << " [done]" << RESET() << std::endl;
    return 0;
}


int run_cpptests()
{
    std::cout << '\n';

    std::vector<testcpp>::iterator it = cpptests.begin();
    for( ; it != cpptests.end(); ++it ) {
        it->compile();

        if ( !it->compile_succeeded() ) {
            std::cout << "           ["
                      << BOLD() << "failure" << RESET() << ": compiler exit status = " 
                      << it->compile_status() << "]" << RESET() << std::endl;
            exit(1);
        }      
    }

    // run tests
    it = cpptests.begin();
    for( ; it != cpptests.end(); ++it ) {
        if (! it->run() )
            continue;

        if ( !it->run_succeeded() ) {
            std::cout << "           ["
                      << BOLD() << "failure" << RESET() << ": binary exit status = " 
                      << it->run_status() << "]" << RESET() << std::endl;
            exit(1);
        }      
    }

    std::cout << BOLD() << "all tests passed successfully!" << RESET() << std::endl;
    return 0;
}


int
main(int argc, char *argv[])
{
    std::string default_rc( getenv("HOME") ); default_rc.append("/.testcpprc");
    std::string target;

    const char * rcfile = default_rc.c_str(); 
    int i;

    while(( i = getopt(argc, argv, "D:I:l:L:p:c:h") ) != EOF) 
        switch (i) 
        {
        case 'l': 
            {
                std::string lib( more::format("-l%1") % std::string(optarg) );
                libraries.push_back(lib);
                break;
            }        
        case 'D': 
            {
                std::string macro( more::format("-D%1") % std::string(optarg) );
                macros.push_back(macro);
                break;
            }        
        case 'I': 
            {
                std::string include( more::format("-I%1") % std::string(optarg) );
                includes.push_back(include);
                break;
            }
 
        case 'L': 
            {
                std::string lib( more::format("-L%1") % std::string(optarg) );
                libraries.push_back(lib);
                break;
            }
        case 'p': 
            {
                if ( strcmp("thread",optarg) ) {
                    fprintf(stderr,"-p%s unknown option!\n", optarg);
                    exit(2);
                }
                libraries.push_back("-pthread");
                break;
            }

        case 'c': rcfile = optarg; break; 
        case 'h': fprintf(stderr,usage,__progname); exit(0);
        case '?': fprintf(stderr, "unknown option!"); exit(1);

        }
    
    argc -= optind;
    argv += optind;

    if (argc == 0) { 
        fprintf(stderr,usage,__progname); 
        exit(0);
    }

    target.assign(argv[0]);

    if ( ! config.parse(rcfile) ) {
        exit(1);
    }

    load_cpptests(target);

    run_cpptests();

    return 0;
}
 
