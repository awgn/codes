/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _FORMAT_HH_
#define _FORMAT_HH_ 

#include <any.hh>       // more!

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>


namespace more { 

    class format
    { 
    public:
        format(const std::string &f)
        : m_format(f),
          m_args()
        {}

        ~format()
        {}

        template <typename T>
        format &
        operator % (T rhs)
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
            for(unsigned int i=0; i < obj.m_format.size();) {

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
                    n += (obj.m_format[i]-'0');
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

#endif /* _FORMAT_HH_ */
