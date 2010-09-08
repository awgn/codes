/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#ifndef _PRINT0x_HPP_
#define _PRINT0x_HPP_ 

#include <iostream>
#include <sstream>
#include <string>
#include <cctype>
#include <stdexcept>
#include <cassert>

namespace more { 
    
    ///////////////////////////////////////////////////////////////////////
    // flags manipulator: example -> as::flag<std::ios::hex>(42)
    // 

    template <std::ios_base::fmtflags Fs, typename Tp>
    struct _flags
    {
        _flags(Tp val)
        : value(val)
        {}
        Tp value;
    };

    template <typename CharT, typename Traits, typename Tp, std::ios_base::fmtflags Fs>
    typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const _flags<Fs,Tp> & rhs)
    {
        std::ostream tmp(out.rdbuf());
        tmp.flags(Fs);
        return tmp << rhs.value;
    }
    
    // helper function...

    template <std::ios_base::fmtflags Fs, typename Tp>
    _flags<Fs,Tp> flags(Tp n)
    {
        return _flags<Fs,Tp>(n);
    }

    ///////////////////////////////////////////////////////////////////////////
    // print: example -> more::print(cout, "%1 %2", std::string("hello"), 
    //                                              this_thread::get_id());
    //

    namespace detail {

        template <typename CharT, typename Traits>
        void stream_on(std::basic_ostream<CharT, Traits> &out, int, int) 
        {
            throw std::runtime_error("%format error%");
        }

        template <typename CharT, typename Traits, typename T, typename ... Ts>
        void stream_on(std::basic_ostream<CharT, Traits> &out, int n, int x, const T &arg0, const Ts& ...args) 
        {
            if (n != x) {
                stream_on(out, n, x+1, args...);
                return;
            }
            out << arg0;
        }
    } // namespace detail

    template <typename CharT, typename Traits, typename T0>
    inline void print(std::basic_ostream<CharT, Traits> &out, const T0 &a0)
    {
        out << a0;
    }

    template <typename CharT, typename Traits, typename ... Ts>
    inline void print(std::basic_ostream<CharT, Traits> &out, const char *fmt, const Ts&... args)
    {
        enum __state { zero, percent, digit } state = zero;
        int n = 0;

        for(const char * p = fmt; *p != '\0'; ++p)
        {
            const char c = *p;
            switch(state)
            {
            case zero: 
                {
                    if(c != '%') {
                        out.put(c); continue;
                    }
                    state = percent; continue;      
                }
            case percent:
                {
                    if(isdigit(c)) {
                        n = (n*10)+(c-'0');
                        state = digit; continue;
                    }
                    if(c == '%')  {
                        out.put('%');
                        state = zero; continue;
                    }
                    throw std::runtime_error("%format error%");
                }
            case digit:
                {
                    if (isdigit(c)) {
                        n = (n*10)+(c-'0');
                        continue;
                    }
                    if (c == '%') {
                        detail::stream_on(out, n, 1, args...);
                        n = 0; state = percent; continue;
                    }
                    detail::stream_on(out, n, 1, args...);
                    out.put(c); 
                    n = 0; state = zero; continue;
                }
            }    
        }
        if (state == percent)
            throw std::runtime_error("%format error %");

        if (state == digit)
            detail::stream_on(out, n, 1, args...);
    }
    
    ///////////////////////////////////////////////////////////////////////////
    // print: example -> more::sprint("%1 %2", std::string("hello"), 
    //                                          this_thread::get_id());
    //
 
    template <typename ... Ts>
    inline std::string sprint(const Ts& ... args)
    {
        std::ostringstream out;
        print(out, args...);
        return out.str();  
    }
 
} // namespace as

#endif /* _PRINT_HPP_ */
