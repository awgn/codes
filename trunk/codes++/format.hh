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

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <cassert>

#include <typecode.hh>

namespace more { 

    class format
    { 
    public:
        format(const std::string &f)
        : _M_format(f),
        _M_args()
        {}

        ~format()
        {}

        template <typename T>
        format &
        operator % (T rhs)
        {
            _M_args.push_back(gt::type(rhs));
            return *this;
        }   

        operator std::string()
        {
            std::stringstream s;
            s << *this;
            return s.str();
        }

        friend std::ostream & 
        operator<<(std::ostream &out, format &obj)
        {
            for(unsigned int i=0; i < obj._M_format.size();) {

                if ( obj._M_format[i] != '%' ) {
                    out << obj._M_format[i++]; 
                    continue;
                }

                if (obj._M_format[++i] == '%') {
                    out << obj._M_format[i++];
                    continue;
                }

                assert( isdigit(obj._M_format[i]) );

                unsigned int n = 0; 
                for (; isdigit(obj._M_format[i]); i++)
                {
                    n *= 10;
                    n += (obj._M_format[i]-'0');
                }

                assert ( n <= obj._M_args.size() );
                gt::type & e = obj._M_args[n-1];

                switch(e.code()) 
                {
#define typecode(xxx, e) gt::xxx:  \
                    out <<  e.get<gt::xxx>(); \
                    break
                    case typecode(_char,e);
                    case typecode(u_char,e);
                    case typecode(short_int,e);
                    case typecode(u_short_int,e);
                    case typecode(_int,e);
                    case typecode(u_int,e);
                    case typecode(long_int,e);
                    case typecode(u_long_int,e);
                    case typecode(long_long_int,e);
                    case typecode(u_long_long_int,e);
                    case typecode(_float,e);
                    case typecode(_double,e);
                    case typecode(long_double,e);
                    case typecode(char_p,e);
                    case typecode(const_char_p,e);
                    case typecode(std_string,e);
                }            

            }

            return out;
        }

    private:
        std::string _M_format;
        std::vector<gt::type> _M_args;

        // non-copyable idiom
        format(const format &);
        format & operator=(const format &);
    };

} // namespace more

#endif /* _FORMAT_HH_ */
