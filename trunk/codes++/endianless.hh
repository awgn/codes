/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef ENDIANLESS_HH
#define ENDIANLESS_HH

#include <arpa/inet.h>
#include <typelist.hh>

namespace more {

    template <typename T> 
    class endianless
    {
    public:
        template <int n>
        struct size2type {
            enum { value = n };
        };

        endianless() 
        : _M_data (T())
        {}

        explicit endianless(const endianless &rhs)
        : _M_data(rhs._M_data)
        {}

        explicit endianless(const T &rhs) 
        : _M_data(hton(rhs, size2type<sizeof(T)>()))
        {}

        endianless &
        operator=(const T &rhs)
        {
            _M_data = hton(rhs, size2type<sizeof(T)>());
            return *this;
        }
        endianless &
        operator=(const endianless &rhs)
        {
            if ( this != &rhs)
                _M_data = rhs._M_data; 
            return *this;
        }

        operator T() const 
        { return ntoh(_M_data, size2type<sizeof(T)>()); }

        const T get() const
        { return _M_data; }

    private:
        template <typename U>
        const U hton(const U n,size2type<2>) const
        { return htons(n); }

        template <typename U>
        const U ntoh(const U n,size2type<2>) const
        { return ntohs(n); }

        template <typename U>
        const U hton(const U n,size2type<4>) const
        { return htonl(n); }

        template <typename U>
        const U ntoh(const U n,size2type<4>) const 
        { return ntohl(n); }

        template <typename U>
        const U hton(const U n,size2type<8>) const
        { return (static_cast<U>(htonl (n)) << 32) + htonl (n >> 32); }

        template <typename U>
        const U ntoh(const U n,size2type<8>) const
        { return (static_cast<U>(ntohl (n)) << 32) + ntohl (n >> 32); }

        typedef TYPELIST(short unsigned int, short int,
                         unsigned int, int,
                         unsigned long int, long int,
                         unsigned long long int, long long int) valid_tlist;

        typename mtp::TL::has_type<valid_tlist, T>::type _M_data;

    };

} // namespace more

#endif /* ENDIANLESS_HH */
