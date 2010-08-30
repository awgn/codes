/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */
 
#ifndef _MORE_ENUMAP_HPP_
#define _MORE_ENUMAP_HPP_ 

#include <iostream>
#include <map>
#include <cstdlib>
#include <string>

// enumap: pure macro/meta/c++ programming trickology.
//

#if __GNUC__ == 4 && __GNUC_MINOR__ >= 3

#define enumap_init(ctor, n) \
    ctor() \
    { more::enumap_line<__COUNTER__+2, __COUNTER__+ n>::init(this); }

#define enumap_entry(s,v) \
    enum { s = v }; \
    \
    static inline const char *\
    _get(more::enumap_tag<v>) \
    { return #s; }; \
    \
    void _set(more::enumap_tag<__COUNTER__>) \
    { direct[ #s ] = v; reverse[v ] = #s; }

#else
    #ifndef NDEBUG
    #warning "with this compiler enumap_init() and enumap_entry() macros must be used in contiguous lines"
    #endif
#define enumap_init(ctor, n) \
    ctor() \
    { more::enumap_line<__LINE__+1, __LINE__+ n>::init(this); }

#define enumap_entry(s,v) \
    enum { s = v }; \
    \
    static inline const char *\
    _get(more::enumap_tag<v>) \
    { return #s; }; \
    \
    void _set(more::enumap_tag<__LINE__>) \
    { direct[ #s ] = v; reverse[v ] = #s; }

#endif

namespace more 
{
    template <int n>
    struct enumap_tag {};

    // is crtp really needed ? Yes man.
    //

    template <typename Q> 
    struct enumap 
    {
        typedef std::map<std::string,int> direct_map;
        typedef std::map<int,std::string> reverse_map;

        // Meyers' singleton: build-on-the-first-use idiom prevents
        //                    linker from removing unused enumap objects

        static const Q & instance()
        { static const Q _ref_; 
            return _ref_; }

        static bool has(const std::string &a)
        { static const Q & r __attribute__((used)) = instance(); 
            return direct.find(a) != direct.end(); }

        static bool has(const int a)
        { static const Q & r __attribute__((used)) = instance(); 
            return reverse.find(a) != reverse.end(); }

        static int eval(const std::string &a)
        { static const Q & r __attribute__((used)) = instance(); 
            auto it = direct.find(a);
            if (it == direct.end()) {
                std::clog << __PRETTY_FUNCTION__  << ": unknown value (" << a << ")" << std::endl;
                return int(); 
            }
            return it->second; }

        static std::string 
        eval(const int a) 
        { static const Q & r __attribute__((used)) = instance(); 
            auto it = reverse.find(a);
            if (it == reverse.end()) {
                std::clog << __PRETTY_FUNCTION__  << ": unknown value [" << a << "]" << std::endl;
                return std::string(); 
            }
            return it->second; }

        template <int K>
        static const char * get()
        { return Q::_get(more::enumap_tag<K>()); }

            
        static direct_map  direct;
        static reverse_map reverse;
    };

    template <typename Q>
    typename enumap<Q>::direct_map enumap<Q>::direct;

    template <typename Q>
    typename enumap<Q>::reverse_map enumap<Q>::reverse;

    template <int a, int b>
    struct enumap_line
    {
        template <typename T>
        static int init(T *that)
        { that->_set(enumap_tag<a>()); 
            return enumap_line<a+1,b>::init(that); }
    };
    template <int n>
    struct enumap_line<n,n>
    {
        template <typename T>
        static int init(T *that)
        { that->_set(enumap_tag<n>());
            return 0; }
    };

}

#endif /* _MORE_ENUMAP_HPP_ */
