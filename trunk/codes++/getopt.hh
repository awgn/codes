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

#include <iostream>
#include <stdexcept>

#include <unistd.h>
#include <getopt.h>

namespace posix {

    class __opt {

        protected:
            const char *optstring;

        public:
            __opt(const char *o=NULL) 
            : optstring(o) 
            { opterr=0;  }

            virtual ~__opt() 
            {}

            virtual int parseopt(int i, char *optarg)=0;
            virtual void parse(int &argc, char ** &argv)=0;
    };

    class getopt : protected __opt {

        public:
            getopt(const char *o=NULL) 
            : __opt(o) 
            { opterr=0; }

            void 
            init(const char *o) 
            { __opt::optstring = o; }

            virtual ~getopt() 
            {}

            virtual int parseopt(int i, char *optarg)=0;

            int 
            c_getopt(int & argc, char ** &argv) 
            { return ::getopt(argc,argv,optstring); }

            void 
            parse(int & argc, char ** &argv) 
            {
                int o;
                while ( (o =::getopt(argc, argv, optstring)) != EOF ) {
                    if ( o == '?' )
                        throw std::runtime_error( 
                                std::string("invalid option -- ") + char(optopt) );
                    if ( o == ':' )
                        throw std::runtime_error( 
                                std::string("option requires an argument -- ") + char(optopt) );
                    parseopt(o,optarg);
                }

                argc -= optind;
                argv += optind;
            }

    };


    class getopt_long : protected __opt {

        const struct option *_M_longopts;
        int *_M_longindex;

        public:

        getopt_long(const char *o=NULL, const struct option *lo=NULL, int *li=NULL) 
        : __opt(o), 
          _M_longopts(lo), 
          _M_longindex(li) 
        {}

        virtual ~getopt_long() 
        {}

        void init(const char *o, const struct option *lo, int *li) 
        { 
            __opt::optstring = o; 
            _M_longopts = lo; 
            _M_longindex = li; 
        }

        virtual int parseopt(int i, char *optarg)=0;

        void 
        parse(int & argc, char ** &argv) 
        {
            int o;
            while ( (o =::getopt_long(argc, argv, optstring, _M_longopts, _M_longindex)) != EOF ) {
                if ( o == '?' )
                    throw std::runtime_error( 
                            std::string("invalid option -- ") + char(optopt) );
                if ( o == ':' )
                    throw std::runtime_error( 
                            std::string("option requires an argument -- ") + char(optopt) );
                parseopt(o,optarg);
            }

            argc -= optind;
            argv += optind;

        }

    };

}


#endif /* GETOPT_HH */
