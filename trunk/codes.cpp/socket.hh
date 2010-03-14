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

#include <sockaddress.hh>           // more!
#include <error.hh>                 // more!

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
            return ::send(_M_fd, buf, len, flags); 
        }

        template <std::size_t N>
        int send(const std::tr1::array<iovec,N> &iov, int flags) const
        { 
            const msghdr msg = { NULL, 0, const_cast<iovec *>(&iov.front()), N, NULL, 0, 0 };    
            return ::sendmsg(_M_fd, &msg, flags); 
        }

        int 
        recv(void *buf, size_t len, int flags) const
        { 
            return ::recv(_M_fd, buf, len, flags); 
        }

        template <std::size_t N>
        int recv(std::tr1::array<iovec,N> &iov, int flags) const
        { 
            msghdr msg = { NULL, 0, &iov.front(), N, NULL, 0, 0 };    
            return ::recvmsg(_M_fd, &msg, flags); 
        }

        int 
        sendto(const void *buf, size_t len, int flags, const sockaddress<FAMILY> &to) const 
        { 
            return ::sendto(_M_fd, buf, len, flags, 
                            reinterpret_cast<const struct sockaddr *>(&to), to.len()); 
        }

        int 
        recvfrom(void *buf, size_t len, int flags, sockaddress<FAMILY> &from) const
        { 
            return ::recvfrom(_M_fd, buf, len, flags, 
                              reinterpret_cast<struct sockaddr *>(&from), &from.len()); 
        }
    
        ////////////////// connect/bind/listen/accept: throw in case of non-blocking socket related errors

        virtual int 
        connect(const sockaddress<FAMILY> &addr)
        { 
            if (::connect(_M_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len()) < 0)
            {
                if (errno != EINPROGRESS && errno != EALREADY)
                    throw more::syscall_error("socket::connect", errno);
                return -1;
            }
            return 0;
        }

        virtual void 
        bind(const sockaddress<FAMILY> &my_addr)
        { 
            if (::bind(_M_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len()) < 0)
               throw more::syscall_error("socket::bind", errno); 
        }

        void 
        listen(int backlog) 
        { 
            if(::listen(_M_fd, backlog) <0)
                throw more::syscall_error("socket::listen", errno);
        }

        int 
        accept(sockaddress<FAMILY> &addr, generic_socket<FAMILY> &remote) 
        {
            int s = ::accept(_M_fd,reinterpret_cast<struct sockaddr *>(&addr), &addr.len());
            if (s < 0) {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                    throw more::syscall_error("socket::accept", errno);
                return -1; 
            }
            remote.close_fd();
            remote._M_fd = s;
            return s;
        }

        ////////////////////////////////////////////

        int 
        getsockname(sockaddress<FAMILY> &name) const
        { 
            return ::getsockname(_M_fd, reinterpret_cast<struct sockaddr *>(&name), &name.len()); 
        }

        int 
        getpeername(sockaddress<FAMILY> &name) const
        { 
            return ::getpeername(_M_fd, reinterpret_cast<struct sockaddr *>(&name), &name.len()); 
        }

        int 
        setsockopt(int level, int optname, const void *optval, socklen_t optlen)
        { 
            return ::setsockopt(_M_fd, level, optname, optval, optlen); 
        }

        int 
        getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
        { 
            return ::getsockopt(_M_fd, level, optname, optval, optlen); 
        }

        int 
        fd() const 
        { 
            return _M_fd; 
        }        

        bool
        is_open() const
        { return _M_fd == -1; }

        void 
        close()
        {
            this->close_fd();
        }

        void 
        init(int type,int protocol=0) 
        {
            close_fd();
            _M_fd = ::socket(FAMILY, type, protocol);
            if (_M_fd == -1) {
                throw more::syscall_error(std::string("socket::init"),errno);
            }
        }

    protected:

        mutable int _M_fd;

        virtual ~generic_socket()
        {
            this->close_fd();
        }

        generic_socket(int s = -1) 
        : _M_fd(s)
        {}

        generic_socket(__socket_type type, int protocol=0)
        : _M_fd(::socket(FAMILY, type, protocol))
        {
            if ( _M_fd == -1) {
                throw more::syscall_error(std::string("socket()"), errno);
            }
        }

        generic_socket(const generic_socket &rhs)
        : _M_fd(-1)
        {
            if ( rhs._M_fd == -1 )
                throw std::runtime_error(std::string("socket(socket &): bad file descriptor"));
            _M_fd = rhs.release();
        }

        generic_socket &
        operator=(const generic_socket &rhs)
        {
            if (this != &rhs) 
            {
                if ( rhs._M_fd == -1 )
                    throw std::runtime_error(std::string("socket::operator=(socket &): bad file descriptor"));
                this->close_fd();
                _M_fd = rhs.release();
            }
            return *this;         
        }

        // close the socket and set fd to -1
        
        void close_fd()
        {
            if (_M_fd != -1) 
            {
                ::close(_M_fd);
                _M_fd = -1;
            }
        }

        // release the socket
        
        int release()
        {
            int __tmp = _M_fd;
            _M_fd = -1;
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
        _M_pathname(),
        _M_bound(false)
        {}

        ~socket() 
        {
            if (!_M_pathname.empty() && _M_bound) {
                ::unlink(_M_pathname.c_str());
            }
        }

        void bind(const sockaddress<PF_UNIX> &my_addr)
        {
            if(::bind(this->_M_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len())<0)
                throw more::syscall_error("socket::bind", errno);
            _M_pathname = my_addr;
        }

        int
        connect(const sockaddress<PF_UNIX> &addr)
        {
            if (::connect(this->_M_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len()) < 0)
            {
                if ( errno != EINPROGRESS && errno != EALREADY)
                    throw more::syscall_error("socket::connect", errno);
                return -1;
            }
            _M_pathname = addr;
            return 0;
        }

    private:
        std::string _M_pathname;   // unix socket
        bool _M_bound;
    };

} // namespace more 

#endif /* SOCKET_HH */

