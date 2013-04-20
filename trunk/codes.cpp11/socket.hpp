/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SOCKET_HPP_
#define _SOCKET_HPP_ 

#include <sys/types.h>       
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sockaddress.hpp>           // more!
#include <buffer.hpp>                // more!
#include <type_traits.hpp>           // more!

#include <string>
#include <array>
#include <cstdint>

namespace more {

    template <int F>
    class generic_socket  
    {
    public:

        generic_socket(generic_socket&& rhs)
        : m_fd(rhs.m_fd)
        {
            rhs.m_fd = -1;
        }

        generic_socket &
        operator=(generic_socket&& rhs)
        {
            if (this != &rhs) 
            {
                this->close_fd();
                m_fd = rhs.m_fd;
                rhs.m_fd = -1;
            }
            return *this;         
        }
 
    protected:

        generic_socket(int s = -1) 
        : m_fd(s)
        {}

        virtual ~generic_socket()
        {
            this->close_fd();
        }
 
        generic_socket(const generic_socket&) = delete;
        generic_socket& operator=(const generic_socket&) = delete;

        generic_socket(__socket_type type, int protocol=0)
        : m_fd(::socket(F, type, protocol))
        {
            if ( m_fd == -1) 
                throw std::system_error(errno, std::generic_category(), "generic_socket");
        }
        
        template <typename Fun, typename Ptr>
        static size_t
        atomic_io(Fun fun, int fd, Ptr _s, size_t n, int flags)
        {
            Ptr s = _s;
            size_t pos = 0;
            ssize_t res;

            while (n > pos) {
                res = fun (fd, s + pos, n - pos, flags);
                switch (res) {
                case -1:
                    if (errno == EINTR || errno == EAGAIN)
                        continue;
                    return 0;
                case 0:
                    errno = EPIPE;
                    return pos;
                default:
                    pos += static_cast<size_t>(res);
                }
            }
            return pos;
        }

    public:

        ///////////////////// I/O methods do not throw ///////////////////// 

        std::ptrdiff_t 
        send(const_buffer buf, int flags) const
        { 
            return ::send(m_fd, buf.addr(), buf.size(), flags); 
        }

        template <typename Cont, typename T = typename std::enable_if<traits::is_vector_like<Cont>::value>::type >
        std::ptrdiff_t
        send(const Cont &iov, int flags) const
        { 
            const msghdr msg = { nullptr, 0, 
                                 const_cast<iovec *>(std::addressof(*std::begin(iov))), 
                                 static_cast<size_t>(std::distance(std::begin(iov), std::end(iov))), 
                                 nullptr, 0, 0 };    
            return ::sendmsg(m_fd, &msg, flags); 
        }

        std::ptrdiff_t 
        recv(mutable_buffer buf, int flags) const
        { 
            return ::recv(m_fd, buf.addr(), buf.size(), flags); 
        }

        template <typename Cont, typename T = typename std::enable_if<traits::is_vector_like<Cont>::value>::type >
        std::ptrdiff_t
        recv(Cont &iov, int flags) const
        { 
            msghdr msg = { nullptr, 0, 
                           std::addressof(*std::begin(iov)), 
                           static_cast<size_t>(std::distance(std::begin(iov), std::end(iov))), 
                           nullptr, 0, 0 };    
            return ::recvmsg(m_fd, &msg, flags); 
        }

        int 
        sendto(const_buffer buf, int flags, const sockaddress<F> &to) const 
        { 
            return ::sendto(m_fd, buf.addr(), buf.size(), flags, &to.c_addr(), to.len()); 
        }

        std::ptrdiff_t 
        recvfrom(mutable_buffer buf, int flags, sockaddress<F> &from) const
        { 
            return ::recvfrom(m_fd, buf.addr(), buf.size(), flags, &from.c_addr(), &from.len()); 
        }
        
        ////////////////// atomic send/recv
    
        void
        send_atomic(const_buffer buf, int flags) const
        {
            if (atomic_io(::send, m_fd, buf.addr(), buf.size(), flags) !=
                    buf.size())
                throw std::system_error(errno, std::generic_category(), "send_atomic");
        }

        void
        recv_atomic(mutable_buffer buf, int flags) const
        {
            if (atomic_io(::recv, m_fd, buf.addr(), buf.size(), flags) !=
                    buf.size())
                throw std::system_error(errno, std::generic_category(), "recv_atomic");
        }
        
        ////////////////// connect/bind/listen/accept: throw when errors occur with non-blocking socket 

        virtual int 
        connect(const sockaddress<F> &addr)
        { 
            if (::connect(m_fd, &addr.c_addr(), addr.len()) < 0)
            {
                if (errno != EINPROGRESS && errno != EALREADY)
                    throw std::system_error(errno, std::generic_category(), "connect");
                return -1;
            }
            return 0;
        }

        virtual void 
        bind(const sockaddress<F> &addr)
        { 
            if (::bind(m_fd, &addr.c_addr(), addr.len()) < 0)
               throw std::system_error(errno, std::generic_category(), "bind");
        }

        void 
        listen(int backlog) 
        { 
            if(::listen(m_fd, backlog) <0)
               throw std::system_error(errno, std::generic_category(), "listen");
        }

        int 
        accept(sockaddress<F> &addr, generic_socket<F> &remote) 
        {
            int s = ::accept(m_fd, &addr.c_addr(), &addr.len());
            if (s < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    throw std::system_error(errno, std::generic_category(), "accept");
                return -1; 
            }

            remote.close_fd();
            remote.m_fd = s;
            return s;
        }

        ////////////////// other methods...

        int 
        getsockname(sockaddress<F> &name) const
        { 
            return ::getsockname(m_fd, &name.c_addr(), &name.len()); 
        }

        int 
        getpeername(sockaddress<F> &name) const
        { 
            return ::getpeername(m_fd, &name.c_addr(), &name.len()); 
        }

        int 
        setsockopt(int level, int optname, const void *optval, socklen_t optlen)
        { 
            return ::setsockopt(m_fd, level, optname, optval, optlen); 
        }

        int 
        getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
        { 
            return ::getsockopt(m_fd, level, optname, optval, optlen); 
        }

        int 
        fd() const 
        { 
            return m_fd; 
        }        

        bool
        is_open() const
        { 
            return m_fd == -1; 
        }

        void 
        close()
        {
            this->close_fd();
        }

    protected: //////////////////////////////////////////////////////////////

        int m_fd;

        void close_fd()
        {
            if (m_fd != -1) 
            {
                ::close(m_fd);
                m_fd = -1;
            }
        }
    };

    //////////////////////////////////////////////
    // generic socket: PF_INET/PF_INET6...
    //

    template <int F> 
    struct socket : public generic_socket<F> 
    {
        socket()
        : generic_socket<F>()
        {}

        socket(__socket_type type, int protocol=0)
        : generic_socket<F>(type,protocol) 
        {}

        socket(socket&& rhs)
        : generic_socket<F>(std::move(rhs))
        {}

        socket& 
        operator=(socket&& rhs)
        {
            generic_socket<F>::operator=(std::move(rhs));
            return *this;
        }
    };

    // PF_UNIX specialization
    //

    template <> 
    struct socket<PF_UNIX> : public generic_socket<PF_UNIX>   
    {
        socket()
        : generic_socket<PF_UNIX>()
        {}

        socket(__socket_type type, int protocol=0)
        : generic_socket<PF_UNIX>(type,protocol)
        , m_pathname()
        , m_bound(false)
        {}

        ~socket() 
        {
            if (!m_pathname.empty() && m_bound) {
                ::unlink(m_pathname.c_str());
            }
        }

        socket(socket&& rhs)
        : generic_socket<PF_UNIX>(std::move(rhs))
        {}

        socket& 
        operator=(socket&& rhs)
        {
            generic_socket<PF_UNIX>::operator=(std::move(rhs));
            return *this;
        }
 
        void bind(const sockaddress<PF_UNIX> &addr)
        {
            if(::bind(this->m_fd, &addr.c_addr(), addr.len())<0)
                throw std::system_error(errno, std::generic_category(), "bind");
            m_pathname = addr.name();
        }

        int
        connect(const sockaddress<PF_UNIX> &addr)
        {
            if (::connect(this->m_fd, &addr.c_addr(), addr.len()) < 0)
            {
                if ( errno != EINPROGRESS && errno != EALREADY)
                    throw std::system_error(errno, std::generic_category(), "connect");
                return -1;
            }
            m_pathname = addr.name();
            return 0;
        }

    private:
        std::string m_pathname;   // unix socket
        bool m_bound;
    };

} // namespace more 


#endif /* _SOCKET_HPP_ */
