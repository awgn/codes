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

#include <sockaddress.hh>
#include <atomicity-policy.hh>
#include <enable_exception_if.hh>
#include <error.hh>

#include <tr1/array>

#ifndef ENABLE_EXCEPTION   
#define ENABLE_EXCEPTION true 
#endif

namespace more {

#ifdef _REENTRANT
    typedef atomicity::GNU_CXX  default_socket_atomicity_policy;    
#else
    typedef atomicity::NONE  default_socket_atomicity_policy;    
#endif

    template <int FAMILY, typename ATOMICITY>
    class base_socket  : public more::enable_exception_if<ENABLE_EXCEPTION> 
    {
    public:

        int 
        send(const void *buf, size_t len, int flags) const
        { return ::send(_M_fd, buf, len, flags); }

        template <std::size_t N>
        int send(const std::tr1::array<iovec,N> &iov, int flags) const
        { 
            const msghdr msg = { NULL, 0, const_cast<iovec *>(&iov.front()), N, NULL, 0, 0 };    
            return ::sendmsg(_M_fd, &msg, flags); 
        }

        int 
        recv(void *buf, size_t len, int flags) const
        { return ::recv(_M_fd, buf, len, flags); }

        template <std::size_t N>
        int recv(std::tr1::array<iovec,N> &iov, int flags) const
        { 
            msghdr msg = { NULL, 0, &iov.front(), N, NULL, 0, 0 };    
            return ::recvmsg(_M_fd, &msg, flags); 
        }

        int 
        sendto(const void *buf, size_t len, int flags, const sockaddress<FAMILY> &to) const 
        { return ::sendto(_M_fd, buf, len, flags, 
                          reinterpret_cast<const struct sockaddr *>(&to), to.len()); }

        int 
        recvfrom(void *buf, size_t len, int flags, sockaddress<FAMILY> &from) const
        { return ::recvfrom(_M_fd, buf, len, flags, 
                            reinterpret_cast<struct sockaddr *>(&from), &from.len()); }

        virtual int 
        connect(const sockaddress<FAMILY> &addr)
        { return ::connect(_M_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len()); }

        virtual int 
        bind(const sockaddress<FAMILY> &my_addr)
        { return ::bind(_M_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len()); }

        int 
        accept(sockaddress<FAMILY> &addr, base_socket<FAMILY, ATOMICITY> &ret) 
        {
            int r = ::accept(_M_fd,reinterpret_cast<struct sockaddr *>(&addr), &addr.len());
            if (r != -1) { 
                typename ATOMICITY::scoped_lock sl(_M_mutex);
                if (ret._M_fd != -1)
                    ::close(ret._M_fd);
                ret._M_fd = r;
            }
            return r;
        }

        int 
        listen(int backlog) 
        { return ::listen(_M_fd, backlog); }

        int 
        getsockname(sockaddress<FAMILY> &name) const
        { return ::getsockname(_M_fd, reinterpret_cast<struct sockaddr *>(&name), &name.len()); }

        int 
        getpeername(sockaddress<FAMILY> &name) const
        { return ::getpeername(_M_fd, reinterpret_cast<struct sockaddr *>(&name), &name.len()); }

        int 
        setsockopt(int level, int optname, const void *optval, socklen_t optlen)
        { return ::setsockopt(_M_fd, level, optname, optval, optlen); }

        int 
        getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
        { return ::getsockopt(_M_fd, level, optname, optval, optlen); }

        const int 
        fd() const 
        { return _M_fd; }        

        int 
        init(int type,int protocol=0) 
        {
            typename ATOMICITY::scoped_lock sl(_M_mutex);

            if (_M_fd != -1) {
                throw_ ( std::runtime_error("base_socket::init : socket already opened"), -1 );
            }

            _M_fd = ::socket(FAMILY, type, protocol);
            if (_M_fd == -1) {
                throw_ ( more::syscall_error(std::string("base_socket::init"),errno), -1 );
            }
            return 0;
        }

    protected:

        mutable int _M_fd;
        typename ATOMICITY::mutex  _M_mutex;

        base_socket() 
        : _M_fd(-1),
          _M_mutex()
        {}

        base_socket(__socket_type type, int protocol=0)
        : _M_fd(::socket(FAMILY, type, protocol)),
          _M_mutex()
        {
            if ( _M_fd == -1) {
                throw_ ( std::runtime_error(std::string("base_socket: bad file descriptor")));
            }
        }

        base_socket(const base_socket &rhs)
        : _M_fd(),
          _M_mutex()
        {
            typename ATOMICITY::scoped_lock sl(_M_mutex);
            _M_fd = rhs._M_fd;
            if ( _M_fd == -1 )
                throw_ ( std::runtime_error(std::string("base_socket(base_socket &): bad file descriptor")));

            rhs._M_fd = -1;
        }

        base_socket &
        operator=(const base_socket &rhs)
        {
            typename ATOMICITY::scoped_lock sl(_M_mutex);
            if (this != &rhs) {
                _M_fd = rhs._M_fd;            
                if ( _M_fd == -1 )
                    throw_ ( std::runtime_error(std::string("base_socket::operator=(base_socket &): bad file descriptor")), *this);

                rhs._M_fd = -1;
            }
            return *this;         
        }

        virtual ~base_socket()
        {
            typename ATOMICITY::scoped_lock sl(_M_mutex);
            if ( _M_fd != -1 ) {
                ::close(_M_fd);
            }
        }

    };

    // generic socket PF_INET/PF_INET6
    //

    template <int FAMILY, typename ATOMICITY = default_socket_atomicity_policy> 
    struct socket : public base_socket<FAMILY, ATOMICITY> 
    {
        socket()
        : base_socket<FAMILY, ATOMICITY>()
        {}

        socket(__socket_type type, int protocol=0)
        : base_socket<FAMILY,ATOMICITY>(type,protocol) 
        {}

    };

    // PF_UNIX specializations...
    //
    template <typename ATOMICITY>
    struct socket<PF_UNIX, ATOMICITY> : public base_socket<PF_UNIX, ATOMICITY>   
    {
        socket()
        : base_socket<PF_UNIX, ATOMICITY>()
        {}

        socket(__socket_type type, int protocol=0)
        : base_socket<PF_UNIX, ATOMICITY>(type,protocol),
        _M_pathname(),
        _M_bound(false)
        {}

        ~socket() 
        {
            if (!_M_pathname.empty() && _M_bound) {
                ::unlink(_M_pathname.c_str());
            }
        }

        int bind(const sockaddress<PF_UNIX> &my_addr)
        {
            int r = ::bind(this->_M_fd,reinterpret_cast<const struct sockaddr *>(&my_addr), my_addr.len());
            if (r != -1)
                _M_pathname = my_addr;
            return r;
        }

        int connect(const sockaddress<PF_UNIX> &addr)
        {
            int r = ::connect(this->_M_fd, reinterpret_cast<const struct sockaddr *>(&addr), addr.len());
            if (r != -1)
                _M_pathname = addr;
            return r;
        }

    private:
        std::string _M_pathname;   // unix socket
        bool _M_bound;
    };

} // namespace more 

#endif /* SOCKET_HH */

