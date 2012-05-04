/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _STATIC_ALLOCATOR_HPP_
#define _STATIC_ALLOCATOR_HPP_ 

#include <stdexcept>
#include <limits>
#include <new>

#ifndef NDEBUG
#include <iostream>
#endif

namespace more { 

    template <typename Tp>
    class static_allocator
    { 
    public:
      typedef size_t            size_type;
      typedef std::ptrdiff_t    difference_type;
      typedef Tp*               pointer;
      typedef const Tp*         const_pointer;
      typedef Tp&               reference;
      typedef const Tp&         const_reference;
      typedef Tp                value_type;

     template<typename Tp1>
        struct rebind
        { typedef static_allocator<Tp1> other; };

        static_allocator(void* _area, size_type _size) throw()
        : m_area(_area),
          m_size(_size)
        {}

        static_allocator(const static_allocator& rhs) throw()
        : m_area(rhs.m_area),
          m_size(rhs.m_size)
        {}

    template<typename Tp1>
        static_allocator(const static_allocator<Tp1>& rhs) throw() 
        : m_area(rhs.m_area),
          m_size(rhs.m_size)
        {}

        ~static_allocator()
        {}

        pointer
        address(reference __x) const { return &__x; }

        const_pointer
        address(const_reference __x) const { return &__x; }

        size_type
        max_size() const throw()
        { return std::numeric_limits<std::size_t>::max() / sizeof(Tp); }

        pointer
        allocate(size_type __n, const void * = 0)
        {
            const size_type __s = __n * sizeof(Tp);
            
#ifndef NDEBUG
            std::cout << "static_allocator<>: this[" << (void *)this << "]->allocate(" << __s << ") bytes..." << std::endl;
#endif
            if ( __s > m_size )
                throw std::out_of_range("allocate");
            
            m_size -= __s;
            pointer a = static_cast<Tp*>(m_area);
            
            m_area = reinterpret_cast<void *>( static_cast<char*>(m_area) + __s);
            return a;
        }

        void
        deallocate(pointer, size_type)
        {}

        void
        construct(pointer __p, const Tp& __val)
        { ::new((void *)__p) value_type(__val); }

        template<typename... _Args>
        void
        construct(pointer __p, _Args&&... __args)
        { ::new((void *)__p) Tp(std::forward<_Args>(__args)...); }

        void
        destroy(pointer __p) {  __p->~Tp(); }

        void *    m_area;
        size_type m_size;
    };

} // namespace more

#endif /* _STATIC_ALLOCATOR_HPP_ */
