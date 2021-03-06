/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_FORMAT_HPP_
#define _MORE_FORMAT_HPP_

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>

#include <any.hpp>       // more!

namespace more {

    class format
    {
    public:
        format(const std::string &f)
        : m_format(f),
          m_args()
        {}

        ~format() = default;

        template <typename T>
        format &
        operator %(T rhs)
        {
            m_args.push_back(any_out(rhs));
            return *this;
        }

        operator std::string()
        {
            std::stringstream s;
            s << *this;
            return s.str();
        }

        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<< (std::basic_ostream<CharT,Traits> &out, const format &obj)
        {
            size_t size = obj.m_format.size();
            for(unsigned int i=0; i < size;) {

                if ( obj.m_format[i] != '%' ) {
                    out << obj.m_format[i++];
                    continue;
                }

                if (obj.m_format[++i] == '%') {
                    out << obj.m_format[i++];
                    continue;
                }

                assert( isdigit(obj.m_format[i]) );

                unsigned int n = 0;
                for (; isdigit(obj.m_format[i]); i++)
                {
                    n *= 10;
                    n += static_cast<unsigned int>(obj.m_format[i]-'0');
                }

                assert ( n <= obj.m_args.size() );

                out << obj.m_args.at(n-1);
            }
            return out;
        }

    private:
        std::string m_format;
        std::vector<any_out> m_args;
    };

} // namespace more

#endif /* _MORE_FORMAT_HPP_ */
