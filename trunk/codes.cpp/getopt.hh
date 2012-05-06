/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef GETOPT_HH
#define GETOPT_HH

#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>

namespace posix {

    class getopt {

        const char *m_optstring;

    public:
        explicit getopt(const char *o) 
        : m_optstring(o) 
        { opterr=0; }

        virtual ~getopt() 
        {}

    protected:
        virtual int parseopt(int i, char *optarg)=0;

    public:

        int 
        c_getopt(int argc, char ** argv) const
        { return ::getopt(argc,argv,m_optstring); }

        void 
        parse(int & argc, char ** &argv) 
        {
            int o;
            while ( (o =::getopt(argc, argv, m_optstring)) != EOF ) {
                if ( o == '?' )
                    throw std::runtime_error(std::string("invalid option -- ") + char(optopt) );
                if ( o == ':' )
                    throw std::runtime_error(std::string("option requires an argument -- ") + char(optopt) );
                parseopt(o,optarg);
            }

            argc -= optind;
            argv += optind;
        }

    };


    class getopt_long {

        const char *m_optstring;
        const struct option *m_longopts;
        int *m_longindex;

    public:

        getopt_long(const char *o, const struct ::option *lo, int *li) 
        : m_optstring(o), 
        m_longopts(lo), 
        m_longindex(li) 
        {}

        virtual ~getopt_long() 
        {}

    protected:
        virtual int parseopt(int i, char *optarg)=0;

    public:

        int 
        c_getopt_long(int argc, char ** argv) const
        { return ::getopt_long(argc,argv,m_optstring,m_longopts, m_longindex); }

        void 
        parse(int & argc, char ** &argv) 
        {
            int o;
            while ( (o =::getopt_long(argc, argv, m_optstring, m_longopts, m_longindex)) != EOF ) {
                if ( o == '?' )
                    throw std::runtime_error(std::string("invalid option -- ") + char(optopt) );
                if ( o == ':' )
                    throw std::runtime_error(std::string("option requires an argument -- ") + char(optopt) );
                parseopt(o,optarg);
            }

            argc -= optind;
            argv += optind;

        }
    };
}

#endif /* GETOPT_HH */
