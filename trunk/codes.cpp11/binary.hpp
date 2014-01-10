/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */
 
#pragma once 

#include <cstdint>
#include <string>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <iostream>

#include <arpa/inet.h> 
#include <endian.h>

namespace more {

    // encode standard types...
    //

    inline void encode(uint64_t n, void *out)
    {
        auto p = reinterpret_cast<uint64_t *>(out);
        * p = htobe64(n); 
    }

    inline void encode(uint32_t n, void *out)
    {
        auto p = reinterpret_cast<uint32_t *>(out);
        * p = htonl(n); 
    }

    inline void encode(uint16_t n, void *out)
    {
        auto p = reinterpret_cast<uint16_t *>(out);
        * p = htons(n); 
    }

    inline void encode(std::string const &s, void *out)
    {
        memcpy(reinterpret_cast<void *>(out), s.data(), s.size());
    }

    inline void encode(const char *s, void *out)
    {
        memcpy(reinterpret_cast<void *>(out), s, std::strlen(s));
    }

    // decode standard types...
    //

    inline void decode(void const *in, uint64_t &n)
    {
        auto p = reinterpret_cast<uint64_t const *>(in);
        n = be64toh(*p);
    }

    inline void decode(void const *in, uint32_t &n)
    {
        auto p = reinterpret_cast<uint32_t const *>(in);
        n = ntohl(*p); 
    }

    inline void decode(void const *in, uint16_t &n)
    {
        auto p = reinterpret_cast<uint16_t const *>(in);
        n = ntohs(*p); 
    }

    inline void decode(void const *in, std::string &str, size_t s)
    {
        str = std::string(reinterpret_cast<const char *>(in), s); 
    }

    inline void decode(void const *in, char *&p, size_t s)
    {
        p = static_cast<char *>(malloc(s+1)); p[s] = '\0';
        std::memcpy(p, in, s);
    }

    // dynamic size_of...
    //

    template <typename Tp>
    inline 
    size_t size_of(Tp)
    {
        return sizeof(Tp);
    }

    inline 
    size_t size_of(std::string const &s)
    {
        return s.size();
    }

    inline 
    size_t size_of(const char *s)
    {
        return std::strlen(s);
    }

    /////////////////////////////////////////////////////////////////////////

    class binary
    {
    public:

        binary(const void * p, size_t n = 0)
        : size_(n)
        , base_(reinterpret_cast<intptr_t>(p))
        , ptr_ (reinterpret_cast<intptr_t>(p))
        {}

        ~binary() = default;

        // get_raw/put_raw
        //

        template <typename T>
        T get_raw()
        {
            auto p = reinterpret_cast<T *>(push_(size_of( T{} )));
            return *p;
        }

        template <typename T>
        void put_raw(const T &v)
        {
            auto p = reinterpret_cast<T *>(push_(size_of( T{} )));
            * p = v;
        }

        // get/put
        //

        template <typename T>
        T get()
        {
            auto in = push_(size_of( T{} ));
            T n; decode(reinterpret_cast<void const *>(in), n);
            return n;
        }
        template <typename T>
        T get(size_t s)
        {
            auto in = push_(s);
            T n; decode(reinterpret_cast<void const *>(in), n, s);
            return n;
        }

        template <typename T>
        void put(T const &n)
        {
            auto out = push_(size_of(n));
            encode(n, reinterpret_cast<void *>(out));
        }

        // get_bytes/put_bytes
        //

        void get_bytes(void * ptr, size_t n)
        {
            auto p = push_(n);
            memcpy(ptr, reinterpret_cast<void *>(p), n);
        }

        void put_bytes(void const * ptr, size_t n)
        {
            auto p = push_(n);
            memcpy(reinterpret_cast<void *>(p), ptr, n);
        }

        // get current pointer:
        //

        void *
        get_ptr() const
        {
            return reinterpret_cast<void *>(ptr_);    
        }

        // bytes: consumed since begin
        //

        size_t 
        bytes() const
        {
            return ptr_ - base_;
        }

        // remaining: bytes 
        //

        size_t
        remaining() const
        {
            return size_ - bytes();
        }

        // empty:
        //

        bool
        empty() const
        {
            return size_ == bytes();
        }

        // skip: bytes
        //

        void 
        skip(size_t n)
        {
            push_(n);
        }

        // put back n bytes
        //

        void
        back(size_t n)
        {
            pop_(n);
        }

        void
        reset()
        {
            ptr_ = base_;
        }

    private:

        intptr_t push_(size_t n)
        {
            auto p = ptr_;
            if ((p + n) > (base_ + size_)) 
                throw std::runtime_error("binary: push overflow");
            ptr_ += n;
            return p;
        }

        intptr_t pop_(size_t n)
        {
            auto p = ptr_;
            if (intptr_t(ptr_ - n) < base_ ) 
                throw std::runtime_error("binary: pop underflow");
            ptr_ -= n;
            return p;
        }

        const size_t   size_;
        const intptr_t base_;
        intptr_t       ptr_;
    };

} // namespace more


