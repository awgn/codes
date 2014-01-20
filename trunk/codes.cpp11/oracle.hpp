/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ORACLE_HPP_
#define _ORACLE_HPP_

#include <cxxabi.hpp> // more!

#include <iostream>
#include <typeinfo>
#include <sstream>
#include <string>
#include <memory>
#include <mutex>


// simple c++0x Oracle
//

struct O
{
    template <typename T, typename CharT, typename Traits>
    static void print_compat(std::basic_ostream<CharT,Traits> &out,const T &elem)
    {
        static std::string last_token;
#ifdef _REENTRANT
        static std::mutex _S_mutex;
        std::lock_guard<std::mutex> _L_(_S_mutex);
#endif
        std::ostringstream ss; ss << elem;
        const std::string &token = ss.str();
        if ( last_token != token )  // flush
        {
            out << token; last_token = std::move(token);
        }
        else {
            out << '.';
        }
    }

    O()                         { print_compat(std::cout," O()"); }
    O(const O &)                { print_compat(std::cout," O(const O&)"); }
    O &operator=(const O &)     { print_compat(std::cout," op=(const O&)"); return *this; }
    ~O()                        { print_compat(std::cout," ~O()"); }
    O(O &&)                     { print_compat(std::cout," O(O&&)"); }
    O &operator=(O &&)          { print_compat(std::cout," op=(O&&)"); return *this; }

    template <typename T> O(T)
    { std::ostringstream ss; ss << " O(" << demangle(typeid(T).name()) << ")";
        print_compat(std::cout,ss.str()); }

    void swap(O &rhs)           { print_compat(std::cout," swap(O,O)"); }
    bool operator<(const O &rhs) const
    {
         print_compat(std::cout," O<O");
         return this < &rhs;
    }
};

template <typename CharT, typename Traits>
typename std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT,Traits> &out, const O & rhs)
{
    std::ostringstream ss; ss << " O[" << (void *)&rhs << "]";
    O::print_compat(out, ss.str());
    return out;
}

#endif /* _ORACLE_HPP_ */
