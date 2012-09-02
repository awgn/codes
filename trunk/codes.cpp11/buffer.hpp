/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _BUFFER_HPP_
#define _BUFFER_HPP_ 

#include <vector>
#include <array>
#include <utility>
#include <string>

// A buffer class, inspiered to that of boost
//

namespace more { 

    struct const_buffer;

    ///////////////////////////////////////////////////////
    // mutable_buffer...

    struct mutable_buffer
    {
        friend struct const_buffer;

        mutable_buffer() = default;

        mutable_buffer(char *addr, size_t numb)
        : m_addr(addr)
        , m_size(numb)
        {}

        mutable_buffer(const mutable_buffer &other)
        : m_addr(other.m_addr)
        , m_size(other.m_size)
        {}

        mutable_buffer &operator=(const mutable_buffer &other)
        {
            m_addr = other.m_addr;
            m_size  = other.m_size;
            return *this;
        }

        // constructors from siblings...
        //
                
        template <typename Alloc>
        mutable_buffer(std::vector<char, Alloc> &vec)
        : m_addr(&vec.front())
        , m_size(vec.size())
        {}

        template <size_t N>
        mutable_buffer(std::array<char, N> &arr)
        : m_addr(arr.data())
        , m_size(N)
        {}

        template <size_t N>
        mutable_buffer(char (&arr)[N])
        : m_addr(arr)
        , m_size(N)
        {}

        const char *
        addr() const
        {
            return m_addr;
        }

        char *
        addr()
        {
            return m_addr;
        }

        size_t
        size() const
        {
            return m_size;
        }

        explicit operator bool() const
        {
            return m_addr != nullptr;
        }

    private:
        char     *m_addr;
        size_t    m_size; 
    };

    ///////////////////////////////////////////////////////
    // const_buffer...
    
    struct const_buffer
    {
        const_buffer() = default;

        const_buffer(const char *addr, size_t numb)
        : m_addr(addr)
        , m_size(numb)
        {}

        const_buffer(const const_buffer &other)
        : m_addr(other.m_addr)
        , m_size(other.m_size)
        {}

        const_buffer & operator=(const const_buffer &other)
        {
            m_addr = other.m_addr;
            m_size  = other.m_size;
            return *this;
        }
        
        
        const_buffer(mutable_buffer const &other)
        : m_addr(other.addr())
        , m_size(other.size())
        {}

        // constructors from siblings...
        //
        
        template <typename Alloc>
        const_buffer(std::vector<char, Alloc> const &v)
        : m_addr(&v.front())
        , m_size(v.size())
        {}

        template <typename Traits, typename Alloc>
        const_buffer(std::basic_string<char,Traits, Alloc> const &s)
        : m_addr(s.data())
        , m_size(s.size())
        {}

        template <size_t N>
        const_buffer(std::array<char, N> const &arr)
        : m_addr(arr.data())
        , m_size(N)
        {}

        template <size_t N>
        const_buffer(const char (&arr)[N])
        : m_addr(arr)
        , m_size(N)
        {}

        const char *
        addr() const
        {
            return m_addr;
        }

        size_t
        size() const
        {
            return m_size;
        }

        explicit operator bool() const
        {
            return m_addr != nullptr;
        }

    private:
        const char *m_addr;
        size_t      m_size; 
    };


} // namespace more

#endif /* _BUFFER_HPP_ */
