/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef ATOMICIO_HH
#define ATOMICIO_HH

#include <tr1/type_traits>
#include <stdexcept>
#include <errno.h>
#include <unistd.h>

namespace more
{
    template <bool v, typename U, typename V>
    struct __select 
    {
        typedef U type;
    };
    template <typename U, typename V>
    struct __select<false, U, V> 
    {
        typedef V type;
    };

    // files: read/write signatures like
    //

    template < typename C, typename T > 
    int atomicio(C callback, int fd, T *buff, size_t n)
    {
        typename __select< std::tr1::is_const<T>::value , const char *,char *>::type p = reinterpret_cast<typeof(p)>(buff);
        size_t res, pos = 0;

        while (n > pos) {
            res = (callback) (fd, p + pos, n - pos);
            switch(res) {
            case -1:
                if ( errno == EINTR || errno == EAGAIN )
                    continue;
                throw std::runtime_error("atoimic_io failure");
            case 0:
                return res;
            default:
                pos += res;
            }
        }
        return pos;
    }

    // socket: send/recv signatures like
    //

    template < typename C, typename T > 
    int atomicio(C callback, int fd, T *buff, size_t n, int flags)
    {
        typename __select< std::tr1::is_const<T>::value , const char *,char *>::type p = reinterpret_cast<typeof(p)>(buff);
        size_t res, pos = 0;

        while (n > pos) {
            res = (callback) (fd, p + pos, n - pos, flags);
            switch(res) {
            case -1:
                if ( errno == EINTR || errno == EAGAIN )
                    continue;
                throw std::runtime_error("atoimic_io failure");
            case 0:
                return res;
            default:
                pos += res;
            }
        }
        return pos;
    }

} // namespace more

#endif /* ATOMICIO_HH */

