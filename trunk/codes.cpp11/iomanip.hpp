/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_IOMANIP_HPP_
#define _MORE_IOMANIP_HPP_

#include <string-utils.hpp>  // more

#include <iostream>
#include <limits>
#include <string>

namespace more {

    // Jusuttis The C++ Standard Library: User-defined manipulators (charter 13)
    //

    // >> ignore_line
    //

    template <class charT, class Traits>
    inline
    std::basic_istream<charT,Traits> &
    ignore_line(std::basic_istream<charT,Traits> &__in)
    {
        __in.ignore(std::numeric_limits<std::streamsize>::max(), static_cast<typename Traits::int_type>(__in.widen('\n')));
        return __in;
    }

    // << spaces(n)
    //

    struct _Spaces { int m_n; };

    static inline _Spaces
    spaces(int __n)
    {
        _Spaces __x;
        __x.m_n = __n;
        return __x;
    }

    template <typename charT, typename Traits>
    inline std::basic_ostream<charT,Traits> &
    operator<<(std::basic_ostream<charT,Traits> &__out, _Spaces __s)
    {
        for(int i=0; i < __s.m_n; i++)
            __out.put(__out.widen(' '));
        return __out;
    }

    // >> basic_token<>
    //

    template<typename CharT, typename Traits, typename Alloc>
    class basic_token
    {
    public:
        basic_token(const std::basic_string<CharT,Traits,Alloc> &delim, bool esc = string_utils::escape_disabled)
        : m_delim(delim),
          m_value(),
          m_esc(esc)
        {}

        ~basic_token()
        {}

        friend inline std::basic_istream<CharT,Traits> &
        operator>>(std::basic_istream<CharT,Traits> &__in, basic_token &rhs)
        {
            std::basic_string<CharT,Traits,Alloc> & __str = rhs;
            more::getline(__in, __str, rhs.m_delim, rhs.m_esc);
            return __in;
        }

        operator const std::basic_string<CharT,Traits,Alloc> &() const
        {
            return m_value;
        }

        operator std::basic_string<CharT,Traits,Alloc> &()
        {
            return m_value;
        }

        const std::basic_string<CharT,Traits,Alloc> &
        str() const
        {
            return m_value;
        }

    protected:
        const std::basic_string<CharT,Traits,Alloc> m_delim;
        std::basic_string<CharT,Traits,Alloc> m_value;
        bool m_esc;
    };

    // >> string_token, wstring_token
    //

    typedef basic_token<std::string::value_type, std::string::traits_type, std::string::allocator_type>
        string_token;

    typedef basic_token<std::wstring::value_type, std::wstring::traits_type, std::wstring::allocator_type>
        wstring_token;


    // >> basic_line (with escape support enabled)
    //

    template<typename CharT, typename Traits, typename Alloc>
    struct basic_line : public basic_token<CharT,Traits,Alloc>
    {
        const char * lf(char)
        {
            return "\n";
        }

        const wchar_t * lf(wchar_t)
        {
            return L"\n";
        }

        basic_line()
        : basic_token<CharT,Traits,Alloc>( lf(CharT()), string_utils::escape_enabled )
        {}

        friend inline std::basic_istream<CharT,Traits> &
        operator>>(std::basic_istream<CharT,Traits> &__in, basic_line &rhs)
        {
            std::basic_string<CharT,Traits,Alloc> & __str = rhs;
            more::getline(__in,__str, rhs.m_delim, rhs.m_esc);
            return __in;
        }
    };

    // >> string_line, wstring_line
    //

    typedef basic_line<std::string::value_type, std::string::traits_type, std::string::allocator_type>
        string_line;

    typedef basic_line<std::wstring::value_type, std::wstring::traits_type, std::wstring::allocator_type>
        wstring_line;

} // namespace more

#endif /* _MORE_IOMANIP_HPP_ */
