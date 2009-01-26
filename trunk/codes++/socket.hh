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

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <sys/uio.h>
#include <errno.h>
#include <err.h>

#include <sockaddress.hh>
#include <tr1/array>

namespace more {

    template <int FAMILY, bool LOG>
    class base_socket  
    {
    public:

        int 
        init(int type,int protocol=0) 
        {
            if (_M_fd != -1) {
                std::clog  << "init: socket already open" << std::endl;
                return -1;
            }

            _M_fd = ::socket(FAMILY, type, protocol);
            if (_M_fd == -1) {
                std::clog  << "init: " << strerror(errno) << std::endl;
            }

            return _M_fd  == -1 ? -1 : 0;
        }

        operator bool()
        { return _M_fd  == -1 ? false : true; }

        int 
        send(const void *buf, size_t len, int flags) 
        { return _log("send", ::send(_M_fd, buf, len, flags)); }

        template <std::size_t N>
        int send(const std::tr1::array<iovec,N> &iov, int flags)
        { 
            const msghdr msg = { NULL, 0, const_cast<iovec *>(&iov.front()), N, NULL, 0, 0 };    
            return _log("sendmsg", ::sendmsg(_M_fd, &msg, flags)); 
        }

        int 
        recv(void *buf, size_t len, int flags) 
        { return _log("recv", ::recv(_M_fd, buf, len, flags)); }

        template <std::size_t N>
        int recv(std::tr1::array<iovec,N> &iov, int flags)
        { 
            msghdr msg = { NULL, 0, &iov.front(), N, NULL, 0, 0 };    
            return _log("recvmsg", ::recvmsg(_M_fd, &msg, flags)); 
        }

        int 
        sendto(const void *buf, size_t len, int flags, const sockaddress<FAMILY> &to) 
        { return _log("sendto",::sendto(_M_fd, buf, len, flags, 
                                        reinterpret_cast<const struct sockaddr *>(&to), to.len())); }

        int 
        recvfrom(void *buf, size_t len, int flags, sockaddress<FAMILY> &from) 
        { return _log("recvfrom", ::recvfrom(_M_fd, buf, len, flags, 
                                             reinterpret_cast<const struct sockaddr *>(&from), &from.len())); }

        virtual int 
        connect(const sockaddress<FAMILY> &addr)
        { return _log("connect", ::connect(_M_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len())); }

        virtual int 
        bind(const sockaddress<FAMILY> &my_addr)
        { return _log("bind", ::bind(_M_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len()) ); }

        int 
        accept(sockaddress<FAMILY> &addr, base_socket<FAMILY, LOG> &ret) 
        {
            int r = _log("accept", ::accept(_M_fd,reinterpret_cast<struct sockaddr *>(&addr), &addr.len()));
            if (r != -1) 
                ret._M_fd = r;
            return r;
        }

        int 
        listen(int backlog) 
        { return _log("listen", ::listen(_M_fd, backlog)); }

        int 
        getsockname(sockaddress<FAMILY> &name) const
        { return _log("getsockname", ::getsockname(_M_fd, reinterpret_cast<const struct sockaddr *>(&name), &name.len())); }

        int 
        getpeername(sockaddress<FAMILY> &name) const
        { return _log("getpeername", ::getpeername(_M_fd, reinterpret_cast<const struct sockaddr *>(&name), &name.len())); }

        int 
        setsockopt(int level, int optname, const void *optval, socklen_t optlen)
        { return _log("setsockopt", ::setsockopt(_M_fd, level, optname, optval, optlen)); }

        int 
        getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
        { return _log("getsockopt", ::getsockopt(_M_fd, level, optname, optval, optlen)); }

        const int 
        fd() const 
        { return _M_fd; }        

    protected:
        int _M_fd;

        base_socket() 
        : _M_fd(-1)
        {}

        base_socket(__socket_type type, int protocol=0)
        : _M_fd(::socket(FAMILY, type, protocol))
        {
            if ( _M_fd == -1) {
                std::clog  << "socket: " << strerror(errno) << std::endl;
            }
        }

        base_socket(const base_socket &);                       // noncopyable
        base_socket &operator=(const base_socket &);            // noncopyable

        virtual ~base_socket()
        {
            if ( _M_fd != -1 ) {
                _log("close",::close(_M_fd));
            }
        }

        int _log(const char *prefix, int ret)
        {
            if ( LOG && ret == -1) 
                std::clog  << prefix << ": " << strerror(errno) << std::endl;
            return ret;
        }
    };

    // generic socket PF_INET/PF_INET6
    //

    template <int FAMILY, bool LOG = true> 
    struct socket : public base_socket<FAMILY, LOG> 
    {
        socket(__socket_type type, int protocol=0)
        : base_socket<FAMILY,LOG>(type,protocol) 
        {}

    };

    // PF_UNIX specializations...
    //
    template <bool LOG>
    struct socket<PF_UNIX, LOG> : public base_socket<PF_UNIX, LOG>   
    {
        socket(__socket_type type, int protocol=0)
        : base_socket<PF_UNIX,LOG>(type,protocol),
        _M_pathname(),
        _M_bound(false)
        {}

        ~socket() 
        {
            if (!_M_pathname.empty() && _M_bound) {
                base_socket<PF_UNIX,LOG>::_log("unlink", ::unlink(_M_pathname.c_str()));
            }
        }

        int bind(const sockaddress<PF_UNIX> &my_addr)
        {
            int r = _log("bind", ::bind(this->_M_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len()));
            if (r != -1)
                _M_pathname = my_addr;
            return r;
        }

        int connect(const sockaddress<PF_UNIX> &addr)
        {
            int r = _log("connect", ::connect(this->_M_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len()));
            if (r != -1)
                _M_pathname = addr;
            return r;
        }

    private:
        std::string _M_pathname;   // unix socket
        bool _M_bound;
    };

} // namespace net

#endif /* SOCKET_HH */

