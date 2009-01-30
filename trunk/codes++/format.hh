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
        operator%(const T & rhs)
        {
            std::stringstream tmp;
            tmp << rhs; _M_args.push_back(tmp.str());
            return *this;
        }   

        friend std::ostream & 
        operator<<(std::ostream &out, const format &obj)
        {
            for(unsigned int i=0; i < obj._M_format.size();) {
                
                if ( obj._M_format[i] == '%' ) {

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
                    out << obj._M_args[n-1];
                    continue;
                }
                
                out << obj._M_format[i++]; 
            }

            return out;
        }

    private:
        std::string _M_format;
        std::vector<std::string> _M_args;

        // non-copyable idiom
        format(const format &);
        format & operator=(const format &);

    };

} // namespace more

#endif /* _FORMAT_HH_ */
