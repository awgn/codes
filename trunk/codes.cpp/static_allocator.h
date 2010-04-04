/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _STATIC_ALLOCATOR_H_
#define _STATIC_ALLOCATOR_H_ 

#include <stdexcept>
#include <limits>
#include <new>

namespace more { 

    template <typename Tp>
    class static_allocator
    { 
    public:
      typedef size_t     size_type;
      typedef ptrdiff_t  difference_type;
      typedef Tp*        pointer;
      typedef const Tp*  const_pointer;
      typedef Tp&        reference;
      typedef const Tp&  const_reference;
      typedef Tp         value_type;

     template<typename Tp1>
        struct rebind
        { typedef static_allocator<Tp1> other; };

        static_allocator(void* _area, size_type _size) throw()
        : _M_area(reinterpret_cast<Tp*>(_area)),
          _M_size(_size)
        {}

        static_allocator(const static_allocator& rhs) throw()
        : _M_area(rhs._M_area),
          _M_size(rhs._M_size)
        {}

    template<typename Tp1>
        static_allocator(const static_allocator<Tp1>& rhs) throw() 
        : _M_area(rhs._M_area),
          _M_size(rhs._M_size)
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
            if ( __n > _M_size )
                throw std::out_of_range("allocate");
            
            _M_size -= __n;
            pointer __a = _M_area;
            _M_area += __n;
            return __a;
        }

        void
        deallocate(pointer __p, size_type)
        {}

        void
        construct(pointer __p, const Tp& __val)
        { ::new((void *)__p) value_type(__val); }

#ifdef __GXX_EXPERIMENTAL_CXX0X__
        template<typename... _Args>
        void
        construct(pointer __p, _Args&&... __args)
        { ::new((void *)__p) _Tp(std::forward<_Args>(__args)...); }
#endif

        void
        destroy(pointer __p) {  __p->~Tp(); }

    private:
        Tp *      _M_area;
        size_type _M_size;
    };

} // namespace more

#endif /* _STATIC_ALLOCATOR_H_ */
