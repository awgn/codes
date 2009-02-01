/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TYPECODE_HH_
#define _TYPECODE_HH_ 

#include <string>
#include <cassert>

namespace more {

    namespace gt {  // generic type

        enum code  
        {
            _char,
            u_char,
            short_int,
            u_short_int,
            _int,
            u_int,
            long_int,
            u_long_int,
            long_long_int,
            u_long_long_int,
            _float,
            _double,
            long_double,
            char_p,
            const_char_p,
            std_string
        };

        //////////////////////////////////////////////////////////////////////////////////////////

        template <typename T> struct type2code;
        template <> struct type2code<char> { enum { value = _char }; };
        template <> struct type2code<unsigned char> { enum { value = u_char }; };
        template <> struct type2code<short int> { enum { value = short_int }; };
        template <> struct type2code<unsigned short int> { enum { value = u_short_int }; };
        template <> struct type2code<int> { enum { value = _int }; };
        template <> struct type2code<unsigned int> { enum { value = u_int }; };
        template <> struct type2code<long int> { enum { value = long_int }; };    
        template <> struct type2code<unsigned long int> { enum { value = u_long_int }; };
        template <> struct type2code<long long int> { enum { value = long_long_int }; };
        template <> struct type2code<unsigned long long int> { enum { value = u_long_long_int }; }; 

        template <> struct type2code<float> { enum { value = _float }; };
        template <> struct type2code<double> { enum { value = _double }; };
        template <> struct type2code<long double> { enum { value = long_double }; };

        template <> struct type2code<char *> { enum { value = char_p }; };              
        template <> struct type2code<const char *> { enum { value = const_char_p }; };              
        template <> struct type2code<std::string> { enum { value = std_string }; };

        //////////////////////////////////////////////////////////////////////////////////////////

        template <int> struct code2type;
        template <> struct code2type<_char> { typedef char type; };
        template <> struct code2type<u_char> { typedef unsigned char type; };
        template <> struct code2type<short_int> { typedef short int type; };
        template <> struct code2type<u_short_int> { typedef unsigned short int type; };
        template <> struct code2type<_int> { typedef int type; };
        template <> struct code2type<u_int> { typedef unsigned int type; };
        template <> struct code2type<long_int> { typedef long int type; };
        template <> struct code2type<u_long_int> { typedef unsigned long int type; };
        template <> struct code2type<long_long_int> { typedef long long int type; };
        template <> struct code2type<u_long_long_int> { typedef unsigned long long int type; };

        template <> struct code2type<_float> { typedef float type; };
        template <> struct code2type<_double> { typedef double type; };
        template <> struct code2type<long_double> { typedef long double type; };

        template <> struct code2type<char_p> { typedef char * type; };
        template <> struct code2type<const_char_p> { typedef const char * type; };
        template <> struct code2type<std_string> { typedef std::string type; };

        //////////////////////////////////////////////////////////////////////////////////////////

        template <int a, int b> struct max
        {
            enum { value = a > b ? a : b };
        };

        class type
        {
            int     _M_type;
            char    _M_storage[ max < sizeof(char), 
                                  max < sizeof(short int),
                                    max < sizeof(int),
                                      max < sizeof(long int),
                                        max < sizeof(long long int),
                                          max < sizeof(float),
                                            max < sizeof(double),
                                              max < sizeof(long double),
                                                max < sizeof(char *), sizeof(std::string)
                                                >::value 
                                              >::value 
                                            >::value 
                                          >::value 
                                        >::value 
                                      >::value 
                                    >::value 
                                  >::value 
                                >::value
                                ];

        public:
            template <typename T>
            type(T value)
            : _M_type(type2code<T>::value), 
              _M_storage()
            { assert( sizeof(T) <= sizeof(_M_storage));
                new (static_cast<void *>(_M_storage)) T(value); 
            }

            int code() const
            { return _M_type; }
            
            template <int n>
            typename code2type<n>::type &
            get() 
            { assert(n == _M_type);
                return * reinterpret_cast< typename code2type<n>::type *>(_M_storage); }

        };
    }
}

#endif /* _TYPECODE_HH_ */
