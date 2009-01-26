/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#include <exception.hh>

extern char *__progname;

namespace more {

    void fatal(const std::string &m) throw() 
    {
        throw more::fatal_error(m);
    }

    void unexpected()
    {
        throw std::bad_exception();
    }

    void terminate() {
        try {
            throw;
        }
        catch(std::exception &e) 
        {
            int status;
            const std::type_info  &ti = typeid(e);
            char *realname = abi::__cxa_demangle(ti.name(), 0, 0, &status);
            std::cerr << __progname << " terminated after throwing a " << realname ;
            std::cerr << " exception (" << e.what() << ")" << std::endl;
            free(realname);
            exit(EX_SOFTWARE);
        }
        catch(...) 
        {
            std::cerr << __progname << " terminated after throwing something...\n";
            exit(EX_SOFTWARE);
        }
    }
}


