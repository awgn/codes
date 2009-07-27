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
#include <iostream>
#include <cstring>

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
        struct __iovec _M_iovec;

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

        ext_buffer(const ext_buffer &b)
        : ext_buffer_base(b._M_iovec.iov_base, b._M_iovec.iov_len)
        {}

        ext_buffer &
        operator=(const ext_buffer &b)
        {
            ext_buffer ret(b);
            std::swap(ret,*this);
            return *this;
        }

        using ext_buffer_base::data;

        void *
        data() 
        { return _M_iovec.iov_base; }

        // iterators 
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

        ext_const_buffer(void * b, size_t s)
        : ext_buffer_base(b,s)
        {}

        ext_const_buffer(const ext_const_buffer &b)
        : ext_buffer_base(b._M_iovec.iov_base, b._M_iovec.iov_len)
        {}

        ext_const_buffer(const ext_buffer &b)
        : ext_buffer_base(b._M_iovec.iov_base, b._M_iovec.iov_len)
        {}

        ext_const_buffer &
        operator=(const ext_const_buffer &b)
        {
            ext_const_buffer ret(b);
            std::swap(ret,*this);
            return *this;
        }        
        
        ext_const_buffer &
        operator=(const ext_buffer &b)
        {
            ext_const_buffer ret(b);
            std::swap(ret,*this);
            return *this;
        }
 
    };


} // namespace more 

#endif /* _EXT_BUFFER_HH_ */
