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

#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>

#include <tr1/type_traits>   
#include <stdexcept>

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

    struct atomic_io {

        // files: read/write signatures like
        //

        template < typename C, typename T > 
        static inline int call(C callback, int fd, T *buff, size_t n)
        {
            typedef typename __select< std::tr1::is_const<T>::value , const char *,char *>::type ptr_type; 
            ptr_type p = reinterpret_cast<ptr_type>(buff);

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

        template < typename C, typename T, typename P1 > 
        static inline int call(C callback, int fd, T *buff, size_t n, P1 p1)
        {
            typedef typename __select< std::tr1::is_const<T>::value , const char *,char *>::type ptr_type; 
            ptr_type p = reinterpret_cast<ptr_type>(buff);

            size_t res, pos = 0;

            while (n > pos) {
                res = (callback) (fd, p + pos, n - pos, p1);
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

        template < typename C, typename T, typename P1, typename P2 > 
        static inline int call(C callback, int fd, T *buff, size_t n, P1 p1, P2 p2)
        {
            typedef typename __select< std::tr1::is_const<T>::value , const char *,char *>::type ptr_type;
            ptr_type p = reinterpret_cast<ptr_type>(buff);
            size_t res, pos = 0;

            while (n > pos) {
                res = (callback) (fd, p + pos, n - pos, p1, p2);
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

        // socket: sendto/recvfrom signatures like
        //

        template < typename C, typename T, typename P1, typename P2, typename P3 > 
        static inline int call(C callback, int fd, T *buff, size_t n, P1 p1, P2 p2, P3 p3)
        {
            typedef typename __select< std::tr1::is_const<T>::value , const char *,char *>::type ptr_type;
            ptr_type p = reinterpret_cast<ptr_type>(buff);
            size_t res, pos = 0;

            while (n > pos) {
                res = (callback) (fd, p + pos, n - pos, p1, p2, p3);
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

    };

    struct interruptible_io {

        template < typename C, typename T > 
        static inline int call(C callback, int fd, T *buff, size_t n)
        {
            return callback(fd, buff, n);
        }

        template < typename C, typename T, typename P1 > 
        static inline int call(C callback, int fd, T *buff, size_t n, P1 p1)
        {
            return callback(fd, buff, n, p1);
        }

        template < typename C, typename T, typename P1, typename P2 > 
        static inline int call(C callback, int fd, T *buff, size_t n, P1 p1, P2 p2)
        {
            return callback(fd, buff, n, p1, p2);
        }

        template < typename C, typename T, typename P1, typename P2, typename P3 > 
        static inline int call(C callback, int fd, T *buff, size_t n, P1 p1, P2 p2, P3 p3)
        {
            return callback(fd, buff, n, p1, p2, p3);
        }

    };

    namespace functor {

        /////////////////////////////////////////
        // I/O functors
        //

        template <typename P>
        struct io_base
        {
        protected:
            P * m_buf;
            size_t m_count;

        public:
            io_base(P *buf, size_t count)
            : m_buf(buf), m_count(count)
            {}

            P *
            data() 
            { return m_buf; }

            size_t
            data_size() const
            { return m_count; }
        };

        template <typename IO = interruptible_io >
        struct read : public io_base<void>
        {
            int m_fd;

            read(int fd, void *buf, size_t count)
            : io_base<void>(buf,count), m_fd(fd)
            {}

            ssize_t operator()() const
            {
                return IO::call(::read,m_fd,m_buf,m_count);
            }
        };

        template <typename IO = interruptible_io >
        struct write : public io_base<const void>
        {
            int m_fd;

            write(int fd, const void *buf, size_t count)
            : io_base<const void>(buf,count), m_fd(fd)
            {}

            ssize_t operator()() const
            {
                return IO::call(::write,m_fd,m_buf,m_count);
            }
        };

        template <typename IO = interruptible_io >
        struct pread : public io_base<void>
        {
            int m_fd;
            off_t   m_offset;

            pread(int fd, void *buf, size_t count, off_t offset)
            : io_base<void>(buf,count), m_fd(fd), m_offset(offset)
            {}

            ssize_t operator()() const
            {
                return IO::call(::pread,m_fd,m_buf,m_count, m_offset);
            }
        };
        template <typename IO = interruptible_io >
        struct pwrite : public io_base<const void>
        {
            int m_fd;
            off_t m_offset;

            pwrite(int fd, const void *buf, size_t count, off_t offset)
            : io_base<const void>(buf,count), m_fd(fd), m_offset(offset)
            {}

            ssize_t operator()() const
            {
                return IO::call(::pwrite,m_fd,m_buf,m_count, m_offset);
            }
        };

        /////////////////////////////////////////////////////////////
        // I/O socket functors
        //

        template <typename IO = interruptible_io >
        struct recv : public io_base<void>
        {
            int m_fd;
            int m_flags;

            recv(int fd, void *buf, size_t count, int flags)
            : io_base<void>(buf,count), m_fd(fd), m_flags(flags)
            {}

            ssize_t operator()() const
            {
                return IO::call(::recv,m_fd,m_buf,m_count, m_flags);
            }
        };

        template <typename IO = interruptible_io >
        struct recvfrom : public io_base<void>
        {
            int m_fd;
            int m_flags;
            struct sockaddr * m_from;
            socklen_t * m_fromlen;

            recvfrom(int fd, void *buf, size_t count, int flags,
                             struct sockaddr *from, socklen_t *fromlen)
            : io_base<void>(buf,count), m_fd(fd), m_flags(flags), m_from(from), m_fromlen(fromlen)
            {}

            ssize_t operator()() const
            {
                return IO::call(::recvfrom,m_fd,m_buf,m_count, m_flags, m_from, m_fromlen);
            }
        };

        template <typename IO = interruptible_io >
        struct send : public io_base<const void>
        {
            int m_fd;
            int m_flags;

            send(int fd, const void *buf, size_t count, int flags)
            : io_base<const void>(buf,count), m_fd(fd), m_flags(flags)
            {}

            ssize_t operator()() const
            {
                return IO::call(::send,m_fd,m_buf,m_count, m_flags);
            }
        };
        template <typename IO = interruptible_io >
        struct sendto : public io_base<const void>
        {
            int m_fd;
            int m_flags;
            struct sockaddr * m_from;
            socklen_t m_tolen;

            sendto(int fd, void *buf, size_t count, int flags,
                           struct sockaddr *from, socklen_t tolen)
            : io_base<const void>(buf,count), m_fd(fd), m_flags(flags), m_from(from), m_tolen(tolen)
            {}

            ssize_t operator()() const
            {
                return IO::call(::sendto,m_fd,m_buf,m_count, m_flags, m_from, m_tolen);
            }
        };

    } // namespace functor
} // namespace more

#endif /* ATOMICIO_HH */

