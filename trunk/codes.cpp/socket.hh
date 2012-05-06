/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef SOCKET_HH
#define SOCKET_HH

#include <sys/types.h>       
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <sockaddress.hh>           // more!
#include <system_error.hh>          // more!

#include <tr1/array>             
#include <string>

namespace more {

    template <int FAMILY>
    class generic_socket  
    {
    public:

        ///////////////////// I/O do not throw ///////////////////// 

        int 
        send(const void *buf, size_t len, int flags) const
        { 
            return ::send(m_fd, buf, len, flags); 
        }

        template <std::size_t N>
        int send(const std::tr1::array<iovec,N> &iov, int flags) const
        { 
            const msghdr msg = { NULL, 0, const_cast<iovec *>(&iov.front()), N, NULL, 0, 0 };    
            return ::sendmsg(m_fd, &msg, flags); 
        }

        int 
        recv(void *buf, size_t len, int flags) const
        { 
            return ::recv(m_fd, buf, len, flags); 
        }

        template <std::size_t N>
        int recv(std::tr1::array<iovec,N> &iov, int flags) const
        { 
            msghdr msg = { NULL, 0, &iov.front(), N, NULL, 0, 0 };    
            return ::recvmsg(m_fd, &msg, flags); 
        }

        int 
        sendto(const void *buf, size_t len, int flags, const sockaddress<FAMILY> &to) const 
        { 
            return ::sendto(m_fd, buf, len, flags, 
                            reinterpret_cast<const struct sockaddr *>(&to), to.len()); 
        }

        int 
        recvfrom(void *buf, size_t len, int flags, sockaddress<FAMILY> &from) const
        { 
            return ::recvfrom(m_fd, buf, len, flags, 
                              reinterpret_cast<struct sockaddr *>(&from), &from.len()); 
        }
    
        ////////////////// connect/bind/listen/accept: throw in case of non-blocking socket related errors

        virtual int 
        connect(const sockaddress<FAMILY> &addr)
        { 
            if (::connect(m_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len()) < 0)
            {
                if (errno != EINPROGRESS && errno != EALREADY)
                    throw more::system_error("socket::connect", errno);
                return -1;
            }
            return 0;
        }

        virtual void 
        bind(const sockaddress<FAMILY> &my_addr)
        { 
            if (::bind(m_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len()) < 0)
               throw more::system_error("socket::bind", errno); 
        }

        void 
        listen(int backlog) 
        { 
            if(::listen(m_fd, backlog) <0)
                throw more::system_error("socket::listen", errno);
        }

        int 
        accept(sockaddress<FAMILY> &addr, generic_socket<FAMILY> &remote) 
        {
            int s = ::accept(m_fd,reinterpret_cast<struct sockaddr *>(&addr), &addr.len());
            if (s < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    throw more::system_error("socket::accept", errno);
                return -1; 
            }
            remote.close_fd();
            remote.m_fd = s;
            return s;
        }

        ////////////////////////////////////////////

        int 
        getsockname(sockaddress<FAMILY> &name) const
        { 
            return ::getsockname(m_fd, reinterpret_cast<struct sockaddr *>(&name), &name.len()); 
        }

        int 
        getpeername(sockaddress<FAMILY> &name) const
        { 
            return ::getpeername(m_fd, reinterpret_cast<struct sockaddr *>(&name), &name.len()); 
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
        { return m_fd == -1; }

        void 
        close()
        {
            this->close_fd();
        }

        void 
        init(int type,int protocol=0) 
        {
            close_fd();
            m_fd = ::socket(FAMILY, type, protocol);
            if (m_fd == -1) {
                throw more::system_error(std::string("socket::init"),errno);
            }
        }

    protected:

        mutable int m_fd;

        virtual ~generic_socket()
        {
            this->close_fd();
        }

        generic_socket(int s = -1) 
        : m_fd(s)
        {}

        generic_socket(__socket_type type, int protocol=0)
        : m_fd(::socket(FAMILY, type, protocol))
        {
            if ( m_fd == -1) {
                throw more::system_error(std::string("socket()"), errno);
            }
        }

        generic_socket(const generic_socket &rhs)
        : m_fd(-1)
        {
            if ( rhs.m_fd == -1 )
                throw std::runtime_error(std::string("socket(socket &): bad file descriptor"));
            m_fd = rhs.release();
        }

        generic_socket &
        operator=(const generic_socket &rhs)
        {
            if (this != &rhs) 
            {
                if ( rhs.m_fd == -1 )
                    throw std::runtime_error(std::string("socket::operator=(socket &): bad file descriptor"));
                this->close_fd();
                m_fd = rhs.release();
            }
            return *this;         
        }

        // close the socket and set fd to -1
        
        void close_fd()
        {
            if (m_fd != -1) 
            {
                ::close(m_fd);
                m_fd = -1;
            }
        }

        // release the socket
        
        int release()
        {
            int __tmp = m_fd;
            m_fd = -1;
            return __tmp;
        }
    
    };

    // generic socket: PF_INET/PF_INET6...
    //

    template <int FAMILY> 
    struct socket : public generic_socket<FAMILY> 
    {
        socket()
        : generic_socket<FAMILY>()
        {}

        socket(__socket_type type, int protocol=0)
        : generic_socket<FAMILY>(type,protocol) 
        {}
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
        : generic_socket<PF_UNIX>(type,protocol),
        m_pathname(),
        m_bound(false)
        {}

        ~socket() 
        {
            if (!m_pathname.empty() && m_bound) {
                ::unlink(m_pathname.c_str());
            }
        }

        void bind(const sockaddress<PF_UNIX> &my_addr)
        {
            if(::bind(this->m_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len())<0)
                throw more::system_error("socket::bind", errno);
            m_pathname = my_addr;
        }

        int
        connect(const sockaddress<PF_UNIX> &addr)
        {
            if (::connect(this->m_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len()) < 0)
            {
                if ( errno != EINPROGRESS && errno != EALREADY)
                    throw more::system_error("socket::connect", errno);
                return -1;
            }
            m_pathname = addr;
            return 0;
        }

    private:
        std::string m_pathname;   // unix socket
        bool m_bound;
    };

} // namespace more 

#endif /* SOCKET_HH */

