/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _EXT_BUFFER_HH_
#define _EXT_BUFFER_HH_ 

#include <sys/uio.h>

#include <atomicio.hh>  // more!

#include <iostream>
#include <cstring>
#include <algorithm>
#include <string>

namespace more {

    class ext_buffer_base
    {
    public:
        typedef size_t                                  size_type;
        typedef const char *                            const_iterator;        
        typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    protected:
        struct __iovec : public ::iovec 
        {
            __iovec(void *base = 0, size_t len = 0)
            {
                iov_base = base;
                iov_len = len;
            }
        };

    public: 
        ext_buffer_base(void * b = 0, size_t s = 0)
        : _M_iovec(b,s)
        {}

        ~ext_buffer_base()
        {}

        // operator[] and at()
        //

        const char &
        operator[](size_type n) const
        { return *(reinterpret_cast<const char *>(_M_iovec.iov_base) + n); }

        const char &
        at(size_type n) const
        { 
            if (_M_iovec.iov_len < n )
                throw std::out_of_range("ext_buffer::at");

            return *(reinterpret_cast<const char *>(_M_iovec.iov_base) + n); 
        }

        // discard
        //

        void discard(size_type n) const 
        {
            if ( n > _M_iovec.iov_len )
                throw std::out_of_range("ext_buffer::discard");

            _M_iovec.iov_base = reinterpret_cast<char *>(_M_iovec.iov_base) + n;  
            _M_iovec.iov_len -= n;
        }

        // iovec support...
        //

        const struct ::iovec *
        operator &() const
        { return static_cast<const struct ::iovec *>(& _M_iovec); }

        operator const struct iovec() const
        { return static_cast<const struct ::iovec &>(_M_iovec); }

        // member access and conversions
        //

        const void *
        data() const 
        { return _M_iovec.iov_base; }

        size_type
        size() const
        { return _M_iovec.iov_len; }

        // const iterators
        //

        const_iterator
        begin() const
        { return reinterpret_cast<char *>(_M_iovec.iov_base); }
        
        const_iterator
        end() const
        { return reinterpret_cast<char *>(_M_iovec.iov_base) + _M_iovec.iov_len; }

        const_reverse_iterator
        rbegin() const
        { return const_reverse_iterator(this->end()); }
        
        const_reverse_iterator
        rend() const
        { return const_reverse_iterator(this->begin()); }

    protected: 
        mutable struct __iovec _M_iovec;

    };

    struct ext_buffer : public ext_buffer_base
    {
        friend class                            ext_const_buffer;
        typedef char *                          iterator;
        typedef std::reverse_iterator<iterator> reverse_iterator;

        // constructors...
        //

        ~ext_buffer()
        {}

        ext_buffer()
        : ext_buffer_base()
        {}

        ext_buffer(void * b, size_t s)
        : ext_buffer_base(b,s)
        {}

        template <typename Fn>
        explicit ext_buffer(Fn cw)
        {
            ssize_t b = cw();
            if (b < 0)
                throw std::runtime_error(std::string("ext_buffer"));

            _M_iovec.iov_base = cw.data();
            _M_iovec.iov_len  = b;
        }

        template <typename Fn>
        ext_buffer(void * b, size_t s, Fn cw)
        {      
            if ( (static_cast<char *>(b) + s) < (static_cast<char *>(cw.data()) + cw.data_size()) )
                throw std::overflow_error("ext_buffer: buffer overflow");

            ssize_t c = cw();
            if (c < 0)
                throw std::runtime_error(std::string("ext_buffer"));

            _M_iovec.iov_base = b;
            _M_iovec.iov_len  = c + (static_cast<char *>(cw.data()) - static_cast<char *>(b));
        }

        template <typename Fn>
        ext_buffer(const ext_buffer &b, Fn cw)
        : ext_buffer_base(0,0)
        { 
            ext_buffer ret(b._M_iovec.iov_base, b._M_iovec.iov_len, cw);
            this->swap(ret);
        } 

        ext_buffer(const ext_buffer &b)
        : ext_buffer_base(b._M_iovec.iov_base, b._M_iovec.iov_len)
        {}

        ext_buffer &
        operator=(const ext_buffer &b)
        {
            ext_buffer ret(b);
            this->swap(ret);
            return *this;
        }

        using ext_buffer_base::data;
        using ext_buffer_base::operator[];
        using ext_buffer_base::at;

        void *
        data() 
        { return _M_iovec.iov_base; }

        char &
        operator[](size_type n)
        { return *(reinterpret_cast<char *>(_M_iovec.iov_base) + n); }

        char &
        at(size_type n)
        { 
            if (_M_iovec.iov_len < n )
                throw std::out_of_range("ext_buffer::at");

            return *(reinterpret_cast<char *>(_M_iovec.iov_base) + n); 
        }

        void commit(size_type n)
        {
            _M_iovec.iov_len += n;
        }

        // non const iterators 
        //

        using ext_buffer_base::begin;
        using ext_buffer_base::end;
        using ext_buffer_base::rbegin;
        using ext_buffer_base::rend;

        iterator
        begin()
        { return reinterpret_cast<char *>(_M_iovec.iov_base); }
        
        iterator
        end()
        { return reinterpret_cast<char *>(_M_iovec.iov_base) + _M_iovec.iov_len; }
 
        reverse_iterator
        rbegin() 
        { return reverse_iterator(this->end()); }
        
        reverse_iterator
        rend() 
        { return reverse_iterator(this->begin()); }
        
        // swap
        //

        void
        swap(ext_buffer &x)
        {
            std::swap(this->_M_iovec.iov_base, x._M_iovec.iov_base);
            std::swap(this->_M_iovec.iov_len, x._M_iovec.iov_len);
        }
     
    };

    struct ext_const_buffer : public ext_buffer_base
    {
        // constructors...
        //

        ~ext_const_buffer()
        {}

        ext_const_buffer()
        : ext_buffer_base()
        {}

        ext_const_buffer(const void * b, size_t s)
        : ext_buffer_base(const_cast<void *>(b),s)
        {}

        ext_const_buffer(const ext_const_buffer &b)
        : ext_buffer_base(b._M_iovec.iov_base, b._M_iovec.iov_len)
        {}

        ext_const_buffer(const ext_buffer &b)
        : ext_buffer_base(b._M_iovec.iov_base, b._M_iovec.iov_len)
        {}

        template <typename Fn>
        explicit ext_const_buffer(Fn cw)
        {
            ssize_t b = cw();
            if (b < 0)
                throw std::runtime_error(std::string("ext_const_buffer"));

            _M_iovec.iov_base = cw.data();
            _M_iovec.iov_len  = b;
        }

        template <typename Fn>
        ext_const_buffer(const void * b, size_t s, Fn cw)
        {            
            if ( (static_cast<const char *>(b) + s) < (static_cast<char *>(cw.data()) + cw.data_size()) )
                throw std::overflow_error("ext_const_buffer: buffer overflow");

            ssize_t c = cw();
            if (c < 0)
                throw std::runtime_error(std::string("ext_const_buffer"));

            _M_iovec.iov_base = const_cast<void *>(b);
            _M_iovec.iov_len  = c + (static_cast<const char *>(cw.data()) - static_cast<const char *>(b));
        }

        template <typename Fn>
        ext_const_buffer(const ext_const_buffer &b, Fn cw)
        : ext_buffer_base(0,0)
        {   
            ext_const_buffer ret(b._M_iovec.iov_base, b._M_iovec.iov_len, cw);
            this->swap(ret);
        } 

        template <typename Fn>
        ext_const_buffer(const ext_buffer &b, Fn cw)
        : ext_buffer_base(0,0)
        {   
            ext_const_buffer ret(b._M_iovec.iov_base, b._M_iovec.iov_len, cw);
            this->swap(ret);
        } 

        ext_const_buffer &
        operator=(const ext_const_buffer &b)
        {
            ext_const_buffer ret(b);
            this->swap(ret);
            return *this;
        }        
        
        ext_const_buffer &
        operator=(const ext_buffer &b)
        {
            ext_const_buffer ret(b);
            this->swap(ret);
            return *this;
        }        
        
        // swap
        //

        void
        swap(ext_const_buffer &x)
        {
            std::swap(this->_M_iovec.iov_base, x._M_iovec.iov_base);
            std::swap(this->_M_iovec.iov_len, x._M_iovec.iov_len);
        }
 
    };

} // namespace more 

namespace std {
   
    // global swap() in std:: namespace

    static inline void
    swap(more::ext_buffer &x, more::ext_buffer &y)
    {
        x.swap(y);
    }
     
    static inline void
    swap(more::ext_const_buffer &x, more::ext_const_buffer &y)
    {
        x.swap(y);
    }
}

#endif /* _EXT_BUFFER_HH_ */
