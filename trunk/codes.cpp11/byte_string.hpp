/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#pragma once 

#include <iostream>
#include <cstring>
#include <string>
#include <stdexcept>

namespace more
{
    template <size_t N>
    struct byte_string
    {
        char value[N];

        byte_string()
        : value()
        {}

        byte_string(const char *s)
        {
            *this = s;
        }
        
        byte_string(const std::string &s)
        {
            *this = s;
        }

        ~byte_string() = default;
        
        byte_string(const byte_string &) = default; 
        byte_string& operator=(const byte_string &) = default; 
        
        byte_string
        operator=(const char *s)
        {
#ifndef NDEBUG
            if (strlen(s) > (N-1))
                std::cerr << "byte_string: overflow for " << s << std::endl;
#endif

            strncpy(value, s, N);
            value[N-1] = 0;
            
            return *this;
        }
        
        byte_string
        operator=(const std::string &s)
        {
#ifndef NDEBUG
            if (s.length() > (N-1))
                std::cerr << "byte_string: overflow for " << s << std::endl;
#endif
            
            strncpy(value, s.c_str(), N);
            value[N-1] = 0;
            
            return *this;
        }
        
        explicit operator bool() const
        {
            return static_cast<bool>(value[0]);
        }

        size_t length() const
        {
            return strlen(value);
        }
    };

    // comparison operators:
    //

    template <size_t N, size_t M>
    inline bool
    operator==(const byte_string<N> &lhs, const byte_string<M> &rhs)
    {
        return strcmp(lhs.value, rhs.value) == 0;
    }
    
    template <size_t N>
    inline bool
    operator==(const byte_string<N> &lhs, const std::string &rhs)
    {
        return strcmp(lhs.value, rhs.c_str()) == 0;
    }
    template <size_t M>
    inline bool
    operator==(const std::string &lhs, const byte_string<M> &rhs)
    {
        return strcmp(lhs.c_str(), rhs.value) == 0;
    }
    
    template <size_t N>
    inline bool
    operator==(const byte_string<N> &lhs, const char *rhs)
    {
        return strcmp(lhs.value, rhs) == 0;
    }
    template <size_t M>
    inline bool
    operator==(const char *lhs, const byte_string<M> &rhs)
    {
        return strcmp(lhs, rhs.value) == 0;
    }
    
    template <size_t N, size_t M>
    inline bool
    operator!=(const byte_string<N> &lhs, const byte_string<M> &rhs)
    {
        return !(lhs == rhs);
    }
    
    template <size_t N>
    inline bool
    operator!=(const byte_string<N> &lhs, const std::string &rhs)
    {
        return !(lhs == rhs);
    }
    template <size_t M>
    inline bool
    operator!=(const std::string &lhs, const byte_string<M> &rhs)
    {
        return !(lhs == rhs);
    }
    
    template <size_t N>
    inline bool
    operator!=(const byte_string<N> &lhs, const char *rhs)
    {
        return !(lhs == rhs);
    }
    template <size_t M>
    inline bool
    operator!=(const char *lhs, const byte_string<M> &rhs)
    {
        return !(lhs == rhs);
    }

    /////////////////////////////////////////////////////////////////

    template <size_t N, size_t M>
    inline bool
    operator<(byte_string<N> const&  lhs, byte_string<M> const&  rhs)
    {
        return strcmp(lhs.value,rhs.value) < 0;
        /* implementation */
    }

    template <size_t N, size_t M>
    inline bool
    operator<=(byte_string<N> const&  lhs, byte_string<M> const&  rhs)
    {
        return !(rhs < lhs);
    }

    template <size_t N, size_t M>
    inline bool
    operator>(byte_string<N> const&  lhs, byte_string<M> const&  rhs)
    {
        return rhs < lhs;
    }

    template <size_t N, size_t M>
    inline bool
    operator>=(byte_string<N> const&  lhs, byte_string<M> const&  rhs)
    {
        return !(lhs < rhs);
    }


    template <typename CharT, typename Traits, size_t N>
    inline typename std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT,Traits> &out, const byte_string<N> & instance)
    {
        if (instance)
            out << '"' << instance.value << '"';
        else
            out << "\"\"";

        return out;
    }

    template <typename CharT, typename Traits, size_t N>
    typename std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT,Traits>& in, byte_string<N> & instance)
    {
        std::string s;
        if (in >> s) 
        {
            auto const size = s.length() - 2;
            
            if (s.front() != '"' ||
                s.back()  != '"' ||
                size > (N-1))
            {
                in.setstate(std::ios::failbit);
                return in;
            }
            
            strcpy(instance.value, std::string(s.begin() + 1, s.end() - 1).c_str());
        }
        return in;
    }

    template <size_t N>
    inline std::string
    show(const byte_string<N> &bs)
    {
        if (bs)
            return '"' + std::string(bs.value) + '"';
        else
            return "\"\"";
    }

} // namespace more


namespace std
{    
    template <size_t N>
    struct hash<more::byte_string<N>>
    {
        static 
        unsigned fnv_hash (const char *key, int len)
        {
            unsigned const char *p = reinterpret_cast<unsigned const char *>(key);
            unsigned h = 2166136261;

            for (int i = 0; i < len; i++ )
                h = ( h * 16777619 ) ^ p[i];

            return h;
        }

        size_t operator()(const more::byte_string<N>& x) const
        {
            return fnv_hash(x.value, x.length());
        }
    };
}
