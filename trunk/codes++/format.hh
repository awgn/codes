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

#include <tr1/memory>
#include <typecode.hh>

namespace more { 

    struct format_arg 
    {
        struct base
        {
            virtual void * get() = 0;
            virtual int type() = 0;
            virtual ~base() {}
        };

        template <typename T>
        struct wrapper : public base
        {
            std::pair<int, T> _M_elem;

            wrapper(int type, T value)
            : _M_elem(std::make_pair(type,value))
            {}

            void *get()
            { return reinterpret_cast<void *>(&_M_elem.second); }

            int type() 
            { return _M_elem.first; }

        };

        template <typename T>
        format_arg(int type, T value)
        : _M_ptr(new wrapper<T>(type,value))
        {}

        void *
        get()
        { return  _M_ptr->get(); }

        int
        type() const
        { return _M_ptr->type(); }

        std::tr1::shared_ptr<base> _M_ptr;
    };

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
        operator % (const T rhs)
        {
            _M_args.push_back(format_arg(type_handling::type2code<T>::value, rhs));
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

                    format_arg & e = const_cast<format_arg &>(obj._M_args[n-1]);

                    switch(e.type()) {
#define typecode(x) type_handling::x:  \
                        out << *reinterpret_cast<type_handling::code2type<type_handling::x>::type *>(e.get()); \
                        break
                        case typecode(_char);
                        case typecode(u_char);
                        case typecode(short_int);
                        case typecode(u_short_int);
                        case typecode(_int);
                        case typecode(u_int);
                        case typecode(long_int);
                        case typecode(u_long_int);
                        case typecode(long_long_int);
                        case typecode(u_long_long_int);
                        case typecode(_float);
                        case typecode(_double);
                        case typecode(long_double);
                        case typecode(char_p);
                        case typecode(const_char_p);
                        case typecode(std_string);
                    default:
                        out << "[unknown type]";
                    }            

                    continue;
                }

                out << obj._M_format[i++]; 
            }

            return out;
        }

    private:
        std::string _M_format;
        std::vector<format_arg> _M_args;

        // non-copyable idiom
        format(const format &);
        format & operator=(const format &);

    };

} // namespace more

#endif /* _FORMAT_HH_ */
