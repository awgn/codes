/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ASYNC_FD_HH_
#define _ASYNC_FD_HH_ 

#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <error.hh>             // more!
#include <noncopyable.hh>       // more!
#include <atomicity-policy.hh>  // more!
#include <functional.hh>        // more!
#include <tr1_type_traits.hh>   // more!

#include <iostream>
#include <algorithm>
#include <iterator>
#include <cassert>
#include <vector>
#include <set>

namespace more { 

#ifdef ASYNC_DEBUG 
    ////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// writev interactive...

    static
    ssize_t writev_interactive(int fd, const struct iovec *_iov, int iovcnt)
    {
        int size = 0;
        struct iovec iov[iovcnt];

        for(int i = 0; i < iovcnt; ++i) {
            iov[i] = _iov[i];
            size  += _iov[i].iov_len;
        }

        std::cerr << "[ sending out iov[" << iovcnt << "] array of " << size << " bytes length. real len? ]";

        int real;
        std::cin  >> real;
        
        if ( real > size )
           real = size;

        for(int i = 0, sum = 0; i < iovcnt; ++i)
        {
            sum += iov[i].iov_len;

            if (real < sum) {
                iov[i].iov_len = ( static_cast<int>(real - sum + iov[i].iov_len) < 0 ? 0 : (real - sum + iov[i].iov_len) );
            }
        }

        return ::writev(fd, iov, iovcnt);
    }

#endif
        

    ////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// async: IO signatures...

    namespace
    {
        struct IO_nonblocking {};
        struct IO_blocking {};        

        /////////////////////////////////////// input signatures 

        typedef ssize_t (*read)(void *, size_t); 
        typedef ssize_t (*readv)(const struct iovec *, int);
        typedef ssize_t (*recv)(void *, size_t, int);
        typedef ssize_t (*recvfrom)(void *, size_t, int, struct sockaddr *, socklen_t *);
        typedef ssize_t (*recvmsg)(struct msghdr *, int);
 
        /////////////////////////////////////// output signatures 

        typedef ssize_t (*write_type)  (int, const void *, size_t);
        typedef ssize_t (*writev_type) (int, const struct iovec *, int);
        typedef ssize_t (*send_type)   (int, const void *, size_t, int);
        typedef ssize_t (*sendto_type) (int, const void *, size_t, int, const struct sockaddr *, socklen_t);
        typedef ssize_t (*sendmsg_type)(int, const struct msghdr *, int);

    }

    //////////////////////////////// async_root class...

    template <typename ATOM>
    class async_root : protected atomicity::emptybase_mutex<ATOM>, private noncopyable
    {
    public:
        int fd() const
        { return _M_fd; }

    protected:
        explicit async_root(int fd)
        : _M_fd(fd)
        {                
            typename ATOM::scoped_lock S( this->static_mutex() );

            if ( _S_set.find(fd) != _S_set.end())
                throw std::logic_error("async_fd<>: fd already in use");

            _S_set.insert(fd); 
        } 

        ~async_root()
        {  
            _S_set.erase(_M_fd);
        }

        int _M_fd;

    private:
        static std::set<int> _S_set;   
    };

    template <typename ATOM> 
    std::set<int> async_root<ATOM>::_S_set; 

    //////////////////////////////// async_base class...

    template <typename R, typename W> 
    class async_base : public async_root<atomicity::DEFAULT> 
    {     
    protected: 
        async_base()
        : async_root<atomicity::DEFAULT>(-1)
        { 
            if ( std::is_same<R,IO_nonblocking>::value ||
                 std::is_same<W,IO_nonblocking>::value )
            {
                this->set_nonblock(_M_flags);    
            } 
            else 
            {
                this->set_block(_M_flags);    
            }               
        }

        explicit async_base(int fd)
        : async_root<atomicity::DEFAULT>(fd)
        {
            if ( std::is_same<R,IO_nonblocking>::value || 
                 std::is_same<W,IO_nonblocking>::value )
            {
                this->set_nonblock(_M_flags);    
            } 
            else 
            {
                this->set_block(_M_flags);    
            }                
        }

        ~async_base()
        {
            try 
            {
                this->set_flags(_M_flags);
            }
            catch(std::exception &e)
            {
                std::clog << "set_flags: " << e.what() << std::endl;
            }
        }

        /////////////////////////////////////// buffered_writes 

        ssize_t
        buffered_write(writev_type, const struct iovec *iov, unsigned int iovcnt, const struct msghdr *, int)
        {
            return ::writev(this->_M_fd, iov, iovcnt);
        }

        ssize_t
        buffered_write(sendmsg_type, const struct iovec *, unsigned int, const struct msghdr *msg, int flags)
        {
            return ::sendmsg(this->_M_fd, msg, flags);
        }

        template <typename F>
        ssize_t
        buffered_write(const struct iovec * iov, unsigned int iovcnt, const struct msghdr *hdr, int flags)
        {
            assert( logical_xor(iov,hdr) ); // iov or hdr are mutually exclusive, yet iov or hdr must be non-null 
           
            int bufsize = _M_buffer.size();

            unsigned int _iovlen = (iov ? iovcnt : hdr->msg_iovlen) + 1;
            iovec        _iov[_iovlen];

            msghdr _hdr;

            // add _M_buffer to the local iovec _iov...
            //

            _iov[0].iov_base = _M_buffer.data();
            _iov[0].iov_len  = bufsize;

            int count = 0;    // total number of bytes of this message
                              // (_M_buffer lenght is not included). 

            if (iov) { /////////////// iovec is provided

                // copy the given array of iovec...
                for(unsigned int i=0; i < iovcnt; ++i)
                {
                    _iov[1+i] = iov[i];
                    count    += iov[i].iov_len;
                }
            }
            else { /////////////// msghdr is provided 

                // override the iovec of the local msghdr...
                _hdr = *hdr;

                _hdr.msg_iov     = _iov;
                _hdr.msg_iovlen  = _iovlen; 

                for(unsigned int i=0; i < hdr->msg_iovlen; ++i)
                {
                    _iov[1+i] = hdr->msg_iov[i];
                    count    += hdr->msg_iov[i].iov_len;
                }
            }

#ifdef ASYNC_DEBUG 
            int bytes = writev_interactive(this->_M_fd, _iov, _iovlen);
#else
            int bytes = this->buffered_write(F(), _iov, _iovlen, &_hdr, flags);
#endif
            if ( (bytes == -1 && (errno != EAGAIN)) || 
                 bytes == 0 /* connection closed by peer */ )   
                throw more::syscall_error(__FUNCTION__);    // an error occurred...

            if ( bytes == (bufsize + count) )   // the whole buffer has been flushed...
            {
                if (bufsize) {
                    _M_buffer.clear();
#ifndef NDEBUG
                    std::cout << "async_fd<" << this->_M_fd << "> #buffer:0" << std::endl;
#endif
                }
                return count;
            }

            std::vector<char> new_buffer;

            int sum = 0;

            for(unsigned int i=0; i < _iovlen; i++)
            {
                sum += _iov[i].iov_len;
                if ( sum > bytes ) {
                    int index = ( static_cast<int>(_iov[i].iov_len)- sum + bytes ) < 0 ?  0 : 
                                ( static_cast<int>(_iov[i].iov_len)- sum + bytes );

                    new_buffer.insert(new_buffer.end(), reinterpret_cast<const char *>(_iov[i].iov_base) + index, 
                                                        reinterpret_cast<const char *>(_iov[i].iov_base) + _iov[i].iov_len );
                }
            }

            swap(_M_buffer,new_buffer);

#ifndef NDEBUG
            std::cout << "async_fd<" << this->_M_fd << "> #buffer:" << this->buffer_size() << std::endl;
#endif
            return count; 
        }  

    public:
        /////////////////////////////////////// fd/buffer_size readers...

        int
        buffer_size() const
        { return _M_buffer.size(); }

        void 
        dump_buffer(std::ostream &out)
        {
            out << "[";
            std::copy(_M_buffer.begin(), _M_buffer.end(), std::ostream_iterator<char>(std::cout, ""));
            out << "]";
        }

        /////////////////////////////////////// poll/wait_for_event on this socket...

        template <short EVENTS>
        short  
        poll(int timeo_msec = 0)
        {
            pollfd pfd = { this->_M_fd, EVENTS, 0 };

            if ( int r = ::poll(&pfd, 1, timeo_msec) >=  0 )
            {
                return r == 0 ? 0 : pfd.revents;    
            }   

            throw more::syscall_error("poll");
            return 0; /* unreachable */ 
        }

        template <short EVENTS>
        bool 
        wait_for_event(int timeo_msec = -1)
        {
           return this->poll<EVENTS>(timeo_msec) & EVENTS; 
        }

        /////////////////////////////////////// block/nonblock/flags...

        void set_nonblock(int & old_flags)
        {
            if ((old_flags = fcntl(this->_M_fd, F_GETFL, 0)) == -1)
                old_flags = 0;
            if ( fcntl(this->_M_fd, F_SETFL, old_flags | O_NONBLOCK) < 0 )
                throw more::syscall_error(__FUNCTION__);
        }

        void set_block(int &old_flags)
        {
            if ((old_flags = fcntl(this->_M_fd, F_GETFL, 0)) == -1)
                old_flags = 0;
            if ( fcntl(this->_M_fd, F_SETFL, old_flags & ~O_NONBLOCK) < 0 )
                throw more::syscall_error(__FUNCTION__);
        }

        void set_flags(int flags)
        {
            if ( fcntl(this->_M_fd, F_SETFL, flags) < 0 )
                throw more::syscall_error(__FUNCTION__);
        }

        int get_flags() const
        {
            int flags;
            if ((flags=fcntl(this->_M_fd, F_GETFL, 0)) == -1)
                throw more::syscall_error(__FUNCTION__);
            return flags;
        }

        /////////////////////////////////////// flush the buffer...

        void flush();

    protected:
        int _M_flags;

        std::vector<char> _M_buffer;
    };


    //////////////////////////////////////////////////////////
    namespace {   

        /////////////////////////////////////// async_io_policy 

        template <typename R, typename W>
        struct async_io_policy
        {
            static
            void block_in(async_base<R,W> &)
            {}

            static
            void block_out(async_base<R,W> &)
            {}
        };

        template <>
        struct async_io_policy<IO_blocking, IO_nonblocking>
        {
            static
            void block_in(async_base<IO_blocking,IO_nonblocking> &ref)
            { 
                if (!ref.wait_for_event<POLLIN>())
                    throw std::runtime_error("POLLIN");
            }

            static
            void block_out(async_base<IO_blocking, IO_nonblocking> &ref)
            {}
        };

        template <>
        struct async_io_policy<IO_nonblocking, IO_blocking>
        {
            static
            void block_in(async_base<IO_nonblocking, IO_blocking> &ref)
            {}

            static
            void block_out(async_base<IO_nonblocking, IO_blocking> &ref)
            {
                if(!ref.wait_for_event<POLLOUT>())
                    throw std::runtime_error("POLLOUT");
            }
        };

     } // namespace ////////////////////////////////////////////////////////////////////////////


    template <typename R, typename W>
    void async_base<R,W>::flush() // basically the atomcio wrapper (see BSD kernel)
    {
        async_io_policy<R,W>::block_out(*this);

        size_t n = _M_buffer.size();
        const char * p = _M_buffer.data();

        size_t res, pos = 0;

        while (n > pos) {
            res = ::write(this->_M_fd, p + pos, n - pos);
            switch(res) {
            case -1:
                if ( errno == EINTR || errno == EAGAIN )
                    continue;
                throw more::syscall_error(__FUNCTION__);    // an error occurred...
            case 0:
                throw more::syscall_error(__FUNCTION__);    // an error occurred...
            default:
                pos += res;
            }
        }

        _M_buffer.clear();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// read strategies...

    template <typename R, typename W>
    class async_read_strategy : public virtual async_base<R, W> // R = IO_blocking
    {
    public:
        ssize_t
        read(void *buf, size_t count) // IO_blocking
        {
            async_io_policy<R,W>::block_in(*this);
            return ::read(async_base<R,W>::_M_fd,buf,count);
        }

        ssize_t 
        readv(const struct iovec *iov, int iovcnt)
        {
            async_io_policy<R,W>::block_in(*this);
            return ::readv(async_base<R,W>::_M_fd, iov, iovcnt);        
        }

        ssize_t 
        recv(void *buf, size_t len, int flags)
        {
            async_io_policy<R,W>::block_in(*this);
            return ::recv(async_base<R,W>::_M_fd, buf, len, flags);
        }

        ssize_t 
        recvfrom(void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
        {
            async_io_policy<R,W>::block_in(*this);
            return ::recvfrom(async_base<R,W>::_M_fd, buf, len, flags, from, fromlen);
        }

        ssize_t 
        recvmsg(struct msghdr *msg, int flags)
        {
            async_io_policy<R,W>::block_in(*this);
            return ::recvmsg(async_base<R,W>::_M_fd, msg, flags);
        }

    };

    template<typename W>
    class async_read_strategy<IO_nonblocking,W> : public virtual async_base<IO_nonblocking, W>
    {
    public:        
        ssize_t
        read(void *buf, size_t count) // IO_nonblocking
        {
            return ::read(async_base<IO_nonblocking,W>::_M_fd,buf,count);
        }

        ssize_t 
        readv(const struct iovec *iov, int iovcnt)
        {
            return ::readv(async_base<IO_nonblocking,W>::_M_fd, iov, iovcnt);        
        }

        ssize_t 
        recv(void *buf, size_t len, int flags)
        {
            return ::recv(async_base<IO_nonblocking,W>::_M_fd, buf, len, flags);
        }

        ssize_t 
        recvfrom(void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
        {
            return ::recvfrom(async_base<IO_nonblocking,W>::_M_fd, buf, len, flags, from, fromlen);
        }

        ssize_t 
        recvmsg(struct msghdr *msg, int flags)
        {
            return ::recvmsg(async_base<IO_nonblocking,W>::_M_fd, msg, flags);
        }

    };

    ////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// write strategies...

    template<typename R, typename W>
    class async_write_strategy : public virtual async_base<R, W>    // W = IO_blocking
    {
    public:        
        
        ssize_t
        write(const void *data, size_t count)
        {
            return ::write(async_base<R,W>::_M_fd, data, count);
        }

        ssize_t 
        writev(const struct iovec *iov, int iovcnt)
        {
            return ::writev(async_base<R,W>::_M_fd, iov, iovcnt);        
        }        
        
        ssize_t 
        send(const void *buf, size_t len, int flags)
        {
            return ::send(async_base<R,W>::_M_fd, buf, len, flags);
        }

        ssize_t 
        sendto(const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
        {
            return ::sendto(async_base<R,W>::_M_fd, buf, len, flags, to, tolen);
        }

        ssize_t 
        sendmsg(const struct msghdr *msg, int flags)
        {
            return ::sendmsg(async_base<R,W>::_M_fd, msg, flags);
        }
    };

    template<typename R>
    class async_write_strategy<R,IO_nonblocking> : public virtual async_base<R, IO_nonblocking>
    {
    public:

        ssize_t
        write(const void *data, size_t count)
        {
            iovec vec[1] = { { const_cast<void *>(data), count } };
            return async_base<R, IO_nonblocking>::template buffered_write<writev_type>(vec, 1, 0, 0);
        }

        ssize_t 
        writev(const struct iovec *iov, int iovcnt)
        {
            return  async_base<R, IO_nonblocking>::template buffered_write<writev_type>(iov, iovcnt, 0, 0);
        }        
        
        ssize_t 
        send(const void *buf, size_t len, int flags)
        {
            return this->sendto(buf, len, flags, NULL, 0);
        }

        ssize_t 
        sendto(const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
        {
            struct iovec iov[1] = { { const_cast<void *>(buf), len } };
            const msghdr msg = { const_cast<struct sockaddr *>(to), tolen, iov, 1, NULL, 0, flags };

            return this->sendmsg(&msg, flags);
        }

        ssize_t 
        sendmsg(const struct msghdr *msg, int flags)
        {
            return this->buffered_write<async_base<R,IO_nonblocking>::sendmsg_type>(0,0,msg,flags);
        }

    };

    ////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////// async_fd class

    template <typename R = IO_blocking, typename W = IO_nonblocking >
    class async_fd : public async_read_strategy<R,W>, public async_write_strategy<R,W>
    {
        public:
            explicit async_fd(int fd)
            : async_base<R,W>(fd)
            {}

            ~async_fd()
            {}
    }; 

    ////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////// free function wrappers...

    template <typename R, typename W>
    inline
    ssize_t read(async_fd<R,W> &bfd, void *buf, size_t count)
    {
        return bfd.read(buf,count);
    }

    template <typename R, typename W>
    inline
    ssize_t write(async_fd<R,W> &bfd, const void *buf, size_t count)
    {
        return bfd.write(buf,count);
    }

    template <typename R, typename W>
    inline
    ssize_t readv(async_fd<R,W> &bfd, const struct iovec *iov, int iovcnt)
    {
        return bfd.readv(iov, iovcnt);        
    }

    template <typename R, typename W>
    inline
    ssize_t writev(async_fd<R,W> &bfd, const struct iovec *iov, int iovcnt)
    {
        return bfd.writev(iov, iovcnt);        
    }

    template <typename R, typename W>
    inline
    ssize_t recv(async_fd<R,W> &bfd, void *buf, size_t len, int flags)
    {
        return bfd.recv(buf, len, flags);
    }

    template <typename R, typename W>
    inline
    ssize_t recvfrom(async_fd<R,W> &bfd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
    {
        return bfd.recvfrom(buf, len, flags, from, fromlen);
    }

    template <typename R, typename W>
    inline
    ssize_t recvmsg(async_fd<R,W> &bfd, struct msghdr *msg, int flags)
    {
        return bfd.recvmsg(msg, flags);
    }

    template <typename R, typename W>
    inline
    ssize_t send(async_fd<R,W> &bfd, const void *buf, size_t len, int flags)
    {
        return bfd.send(buf, len, flags);
    }

    template <typename R, typename W>
    inline
    ssize_t sendto(async_fd<R,W> &bfd, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen)
    {
        return bfd.sendto(buf, len, flags, to, tolen);
    }

    template <typename R, typename W>
    inline
    ssize_t sendmsg(async_fd<R,W> &bfd, const struct msghdr *msg, int flags)
    {
        return bfd.sendmsg(msg, flags);
    }

} // namespace more

#endif /* _ASYNC_FD_HH_ */
