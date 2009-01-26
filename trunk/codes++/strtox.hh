/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef STRTOX_HH
#define STRTOX_HH

#include <stdlib.h>
#include <errno.h>
#include <stdexcept>
#include <string>

namespace more {

    template <typename R, typename F, F fun>
    static inline R strtox(const std::string &s, int base) 
    throw(std::runtime_error,std::range_error) 
    {
        char *endptr;
        errno = 0;
     
        R r = fun(s.c_str(), &endptr, base);
        if (errno == ERANGE)
            throw std::range_error("strtox");

        if (errno != 0 && r == 0) 
            throw std::runtime_error("strtox");

        if (endptr == s.c_str())
            throw std::runtime_error("strtox: no digits were found!");
        
        if (*endptr != '\0')   
            throw std::runtime_error("strtox: further characters after number...");

        return r;
    }

    template <typename R, typename F, F fun>
    static inline R strtox(const std::string &s) 
    throw(std::runtime_error,std::range_error) 
    {
        char *endptr;
        errno = 0;
     
        R r = fun(s.c_str(), &endptr);
        if (errno == ERANGE)
            throw std::range_error("strtox");

        if (endptr == s.c_str())
            throw std::runtime_error("strtox: no digits were found!");
        
        if (*endptr != '\0')   
            throw std::runtime_error("strtox: further characters after number...");

        return r;
    }

    typedef long int (*strtol_t) (const char *, char **,int);
    typedef long long int (*strtoll_t)(const char *, char **,int);
    typedef unsigned long int (*strtoul_t) (const char *, char **, int ); 
    typedef unsigned long long int (*strtoull_t)(const char *, char **,int);
    typedef float (*strtof_t)(const char *, char **);
    typedef double (*strtod_t)(const char *, char **);
    typedef long double (*strtold_t)(const char *, char **);

    static inline long int strtol(const std::string &s, int base = 0)
    throw(std::runtime_error,std::range_error)
    {
        return strtox<long int, strtol_t, ::strtol>(s,base); 
    }
    static inline long long int strtoll(const std::string &s, int base = 0)
    throw(std::runtime_error,std::range_error)
    {
        return strtox<long long int, strtoll_t, ::strtoll>(s,base); 
    }
    static inline unsigned long int strtoul(const std::string &s, int base = 0)
    throw(std::runtime_error,std::range_error)
    {
        return strtox<unsigned long int, strtoul_t, ::strtoul>(s,base); 
    }
    static inline unsigned long long int strtoull(const std::string &s, int base = 0)
    throw(std::runtime_error,std::range_error)
    {
        return strtox<unsigned long long int, strtoull_t, ::strtoull>(s,base); 
    }
    static inline float strtof(const std::string &s)
    throw(std::runtime_error,std::range_error)
    {
        return strtox<float, strtof_t, ::strtof>(s); 
    }
    static inline double strtod(const std::string &s)
    throw(std::runtime_error,std::range_error)
    {
        return strtox<double, strtod_t, ::strtod>(s); 
    }
    static inline long double strtold(const std::string &s)
    throw(std::runtime_error,std::range_error)
    {
        return strtox<long double, strtold_t, ::strtold>(s); 
    }
}

#endif /* STRTOX_HH */
