/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef MMAP_HH
#define MMAP_HH

#include <sys/mman.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <static_assert.hh>         // more!
#include <mtp.hh>                   // more!
#include <system_error.hh>          // more!

#include <stdexcept>

namespace more {

    // is_pow2
    template <int N>
    struct is_pow2
    {
        enum { value = ((N & (N - 1)) == 0) };
    };
    template<>
    struct is_pow2<0>
    {
        enum { value = 0 };
    };

    // mapping traits
    template <int P> struct mapping_traits;
    template <> 
    struct mapping_traits<PROT_READ> 
    { 
        enum { file_mode = O_RDONLY };
        enum { const_map = true };
    };
    template <> 
    struct mapping_traits<PROT_WRITE>
    { 
        enum { file_mode = O_RDWR };
        enum { const_map = false };
    };
    template <> 
    struct mapping_traits<PROT_EXEC> 
    { 
        enum { file_mode = O_RDONLY };
        enum { const_map = true };
    };
    template <> 
    struct mapping_traits<PROT_READ|PROT_WRITE> 
    { 
        enum { file_mode = O_RDWR };
        enum { const_map = false };
    };
    template <> 
    struct mapping_traits<PROT_READ|PROT_EXEC> 
    { 
        enum { file_mode = O_RDONLY };
        enum { const_map = true };
    };
    template <> 
    struct mapping_traits<PROT_WRITE|PROT_EXEC> 
    { 
        enum { file_mode = O_RDWR };
        enum { const_map = false };
    };
    template <> 
    struct mapping_traits<PROT_READ|PROT_WRITE|PROT_EXEC> 
    { 
        enum { file_mode = O_RDWR };
        enum { const_map = false };
    };

    template <int P, int F> 
    class mmap {

        mutable void *  m_addr;
        mutable size_t  m_length;
        mutable bool    m_mapped;
        mutable off_t   m_offset;
        mutable int     m_fd;

        mmap(const mmap &);
        mmap &operator=(const mmap &);

    public:

        mmap(const char *pathname, int len = -1)
        : m_addr(0),
          m_length(len),
          m_mapped(false),
          m_offset(0),
          m_fd(-1)
        {
            static_assert( F & (MAP_PRIVATE|MAP_SHARED), 
                           flags_contains_neither_map_private_or_map_shared);
            static_assert( !(F & (MAP_PRIVATE|MAP_SHARED)) || is_pow2<F & (MAP_PRIVATE|MAP_SHARED)>::value, 
                           flags_contains_both_map_private_and_map_shared);

            if (pathname) {
                m_fd = ::open(pathname, mapping_traits<P>::file_mode);
                if (m_fd == -1)
                    throw more::system_error("coundn't open file");
            }
        }

        const mmap &
        set_offset(off_t o) const
        { m_offset = o; return *this; }

        const mmap &
        set_addr(off_t a) const
        { m_addr = a; return *this; }

        typename mtp::select<mapping_traits<P>::const_map, const void *, void *>::type 
        operator()()
        {
            if (m_mapped)
                return m_addr;

            struct stat sb;

            if ( m_fd != -1 && m_length == size_t(-1) ) {
                if ( fstat(m_fd, &sb) == -1 )
                    throw more::system_error("coundn't fstat file");
                if (!S_ISREG(sb.st_mode)) 
                    throw more::system_error("bad file to mmap");
                m_length = sb.st_size-m_offset;
            }

            m_addr = ::mmap(m_addr, m_length, P, F, m_fd, m_offset);
            if ( m_addr == MAP_FAILED )
                throw more::system_error("mmap");

            if (m_fd != -1 && ::close(m_fd) == -1 )
                throw more::system_error("close");

            m_mapped = true;
            return m_addr;
        }

        const void * 
        operator()() const
        {
            if ( !mapping_traits<P>::const_map )
                throw more::system_error("PROT_WRITE on const object");

            return const_cast<mmap *>(this)->operator()(); 
        }
        
        size_t
        length() const
        { return m_length; }

        ~mmap()
        {
            if ( m_addr && m_addr != MAP_FAILED )
                if ( ::munmap(m_addr, m_length) == -1 )
                    throw more::system_error("munmap");
        }

    };

} // namespace more

#endif /* MMAP_HH */
