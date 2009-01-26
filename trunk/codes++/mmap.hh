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

#include <stdexcept>
#include <exception.hh>

#include <static_assert.hh>
#include <mtp.hh>

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

        mutable void *  _M_addr;
        mutable size_t  _M_length;
        mutable bool    _M_mapped;
        mutable off_t   _M_offset;
        mutable int     _M_fd;

        mmap(const mmap &);
        mmap &operator=(const mmap &);

    public:

        mmap(const char *pathname, int len = -1)
        : _M_addr(0),
          _M_length(len),
          _M_mapped(false),
          _M_offset(0),
          _M_fd(-1)
        {
            static_assert( F & (MAP_PRIVATE|MAP_SHARED), 
                           flags_contains_neither_map_private_or_map_shared);
            static_assert( !(F & (MAP_PRIVATE|MAP_SHARED)) || is_pow2<F & (MAP_PRIVATE|MAP_SHARED)>::value, 
                           flags_contains_both_map_private_and_map_shared);

            if (pathname) {
                _M_fd = ::open(pathname, mapping_traits<P>::file_mode);
                if (_M_fd == -1)
                    throw more::syscall_error("coundn't open file");
            }
        }

        const mmap &
        set_offset(off_t o) const
        { _M_offset = o; return *this; }

        const mmap &
        set_addr(off_t a) const
        { _M_addr = a; return *this; }

        typename mtp::select<mapping_traits<P>::const_map, const void *, void *>::type 
        operator()()
        {
            if (_M_mapped)
                return _M_addr;

            struct stat sb;

            if ( _M_fd != -1 && _M_length == size_t(-1) ) {
                if ( fstat(_M_fd, &sb) == -1 )
                    throw more::syscall_error("coundn't fstat file");
                if (!S_ISREG(sb.st_mode)) 
                    throw more::syscall_error("bad file to mmap");
                _M_length = sb.st_size-_M_offset;
            }

            _M_addr = ::mmap(_M_addr, _M_length, P, F, _M_fd, _M_offset);
            if ( _M_addr == MAP_FAILED )
                throw more::syscall_error("mmap");

            if (_M_fd != -1 && ::close(_M_fd) == -1 )
                throw more::syscall_error("close");

            _M_mapped = true;
            return _M_addr;
        }

        const void * 
        operator()() const
        {
            if ( !mapping_traits<P>::const_map )
                throw std::runtime_error("PROT_WRITE on const object");

            return const_cast<mmap *>(this)->operator()(); 
        }
        
        size_t
        length() const
        { return _M_length; }

        ~mmap()
        {
            if ( _M_addr && _M_addr != MAP_FAILED )
                if ( ::munmap(_M_addr, _M_length) == -1 )
                    throw more::syscall_error("munmap");
        }

    };

} // namespace more

#endif /* MMAP_HH */
