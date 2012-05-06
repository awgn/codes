/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _BUFFER_HH_
#define _BUFFER_HH_ 

#include <memory>
#include <cassert>
#include <algorithm>

namespace more { 

    // a dynamic, fixed-size buffer container inspired to the GNU std::vector
    //

    template <typename _Tp, typename _Alloc = std::allocator<_Tp> > 
    class buffer
    {
    private:
        typedef typename _Alloc::template rebind<_Tp>::other _Tp_alloc_type;

        struct _Vector_impl 
        : public _Tp_alloc_type 
        {
            _Tp *       m_buffer;
            std::size_t m_size;
            _Tp *       m_begin;
            _Tp *       m_end;
            _Vector_impl(_Tp_alloc_type const& __a) 
            : _Tp_alloc_type(__a), m_buffer(0), m_size(0), m_begin(0), m_end(0)
            {}

        } m_Vector_impl;

    public:
        typedef _Tp                                     value_type;
        typedef _Tp *                                   iterator;
        typedef const _Tp *                             const_iterator;
        typedef std::reverse_iterator<iterator>         reverse_iterator;
        typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
        typedef _Tp &                                   reference;
        typedef const _Tp &                             const_reference;
        typedef std::size_t                             size_type;
        typedef std::ptrdiff_t                          difference_type;
        typedef _Alloc                                  allocator_type;

        // constructors
        //

        buffer(size_type __n, const value_type& __value = value_type(), const allocator_type& __a = allocator_type())
        : m_Vector_impl(__a)
        {
            this->m_Vector_impl.m_buffer = this->m_Vector_impl.allocate(__n);
            this->m_Vector_impl.m_size = __n;

            std::__uninitialized_fill_n_a(this->m_Vector_impl.m_buffer, __n, __value, const_cast<allocator_type &>(__a));
            m_Vector_impl.m_begin = m_Vector_impl.m_end = this->m_Vector_impl.m_buffer; }

        ~buffer()
        {  std::_Destroy(this->m_Vector_impl.m_buffer, this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size, this->m_Vector_impl); 
            m_Vector_impl.deallocate(this->m_Vector_impl.m_buffer, this->m_Vector_impl.m_size); 
        }

        // assignment and copy constructor
        //
        buffer(const buffer & rhs)
        : m_Vector_impl(rhs.m_Vector_impl)
        {
            m_Vector_impl.m_buffer = m_Vector_impl.m_begin = m_Vector_impl.m_end = m_Vector_impl.allocate(rhs.max_size());
            m_Vector_impl.m_size = rhs.max_size();
            std::__uninitialized_copy_a(rhs.begin(), rhs.end(), this->m_Vector_impl.m_begin, 
                                        const_cast<typename buffer<_Tp,_Alloc>::_Vector_impl &>(rhs.m_Vector_impl));
            this->commit(rhs.end()-rhs.begin());
        }

       buffer& operator= (const buffer& rhs) 
        { buffer<_Tp, _Alloc> tmp(rhs);
            this->swap(tmp); 
            return *this; }

        // iterators of data...
        // 
        iterator 
        begin() 
        { return this->m_Vector_impl.m_begin; }

        const_iterator 
        begin() const 
        { return this->m_Vector_impl.m_begin; }

        iterator 
        end() 
        { return this->m_Vector_impl.m_end; }

        const_iterator 
        end() const 
        { return this->m_Vector_impl.m_end; }

        reverse_iterator
        rbegin()
        { return reverse_iterator(this->end()); }

        const_reverse_iterator
        rbegin() const
        { return const_reverse_iterator(this->end()); }

        reverse_iterator
        rend()
        { return reverse_iterator(this->begin()); }

        const_reverse_iterator
        rend() const
        { return const_reverse_iterator(this->begin()); }

        // return the number of elements in the buffer
        //
        size_type size() const 
        { return this->m_Vector_impl.m_end - this->m_Vector_impl.m_begin; }

        // return the max. size available for appending data
        //
        size_type capacity() const
        { return this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size - this->m_Vector_impl.m_end; } 

        // return the max. size available for inserting data 
        //
        size_type reverse_capacity() const
        { return this->m_Vector_impl.m_begin - this->m_Vector_impl.m_buffer; } 

        // return the max. size of buffer
        size_type max_size() const
        { return this->m_Vector_impl.m_size; }

        // flush data of buffer...
        //
        void clear()
        {   // optimized away for trivial descrutors
            std::_Destroy(this->m_Vector_impl.m_begin, this->m_Vector_impl.m_end, this->m_Vector_impl);
            this->m_Vector_impl.m_end = this->m_Vector_impl.m_begin; }

        // flush the whole buffer, reset begin() and end()...
        //
        void reset()
        { std::_Destroy(this->m_Vector_impl.m_begin, this->m_Vector_impl.m_begin + this->m_Vector_impl.m_size, this->m_Vector_impl);
            this->m_Vector_impl.m_begin = this->m_Vector_impl.m_buffer; 
            this->m_Vector_impl.m_end = this->m_Vector_impl.m_buffer; }

        // commit n-elements (following end()) after a raw-write...  
        //
        void commit(size_type n)
        { this->m_Vector_impl.m_end += std::min(this->capacity(), n); }

        // discard n-elements from begin()
        //
        void discard(size_type n)
        { n = std::min(this->size(),n);
            // optimized away for trivial descrutors
            std::_Destroy(this->m_Vector_impl.m_begin, this->m_Vector_impl.m_begin + n, this->m_Vector_impl);
            this->m_Vector_impl.m_begin += n; }

        // return true if buffer is empty
        //
        bool empty() const
        { return !(this->m_Vector_impl.m_end-this->m_Vector_impl.m_begin); }

        // operator[]: element access
        //
        reference
        operator[](size_type i)
        { return this->m_Vector_impl.m_begin[i]; }

        const_reference
        operator[](size_type i) const
        { return this->m_Vector_impl.m_begin[i]; }

        // front element
        //
        reference front()
        { return *begin(); }

        const_reference front() const
        { return *begin(); }

        // back element
        //
        reference back()
        { return *(end()-1); }

        const_reference back() const
        { return *(end()-1); }

        // direct access to data, for reading 
        // ie: write (fd, buf.data_read(), bytes); 
        //     buf.discard(bytes);
        //

        const _Tp* 
        data_read() const 
        { return this->m_Vector_impl.m_begin; }

        // direct access to data, for writing 
        // ie: n = read(fd, buf.data_write(), bytes); 
        //     buf.commit(n);
        //

        _Tp* 
        data_write() 
        { return this->m_Vector_impl.m_end; }

        // swap 
        //
        void 
        swap (buffer& rhs) 
        { std::swap(this->m_Vector_impl.m_buffer, rhs.m_Vector_impl.m_buffer);
            std::swap(this->m_Vector_impl.m_size, rhs.m_Vector_impl.m_size);
            std::swap(this->m_Vector_impl.m_begin, rhs.m_Vector_impl.m_begin);
            std::swap(this->m_Vector_impl.m_end, rhs.m_Vector_impl.m_end); }
 
        // push/pop
        //
        void push_back(const _Tp &e)
        { if ( !(this->m_Vector_impl.m_end < this->m_Vector_impl.m_buffer+this->m_Vector_impl.m_size) ) {
                std::__throw_out_of_range("buffer::push_back");
            }
            *(this->m_Vector_impl.m_end++) = e; 
        }

        void pop_back()
        { if ( !(this->m_Vector_impl.m_end > this->m_Vector_impl.m_begin) ) {
                std::__throw_out_of_range("buffer::pop_back");
            }
            --this->m_Vector_impl.m_end;
            this->m_Vector_impl.destroy(this->m_Vector_impl.m_end);
         }

        void push_front(const _Tp &e)
        { if ( !(this->m_Vector_impl.m_begin > this->m_Vector_impl.m_buffer) ) {
                std::__throw_out_of_range("buffer::push_front");
            }
            *(--this->m_Vector_impl.m_begin) = e;
        }

        void pop_front()
        { if ( !(this->m_Vector_impl.m_begin < this->m_Vector_impl.m_end) ) {
                std::__throw_out_of_range("buffer::pop_front");
            }
            this->m_Vector_impl.destroy(this->m_Vector_impl.m_begin++);
        }   

        // erase data from buffer...
        //
        iterator
        erase(iterator _start, iterator _end)
        { if ( _start <  this->m_Vector_impl.m_begin &&
                 _end   <= this->m_Vector_impl.m_begin ) {
                return this->m_Vector_impl.m_begin;
            }
            if ( _start <= this->m_Vector_impl.m_begin &&
                 _end   <= this->m_Vector_impl.m_end ) {
                std::_Destroy(this->m_Vector_impl.m_begin, _end, this->m_Vector_impl);
                this->m_Vector_impl.m_begin = _end;
                return this->m_Vector_impl.m_begin;
            }
            if ( _start <  this->m_Vector_impl.m_end &&
                 _end   <  this->m_Vector_impl.m_end ) {
                std::copy(_end, this->m_Vector_impl.m_end, _start);
                std::_Destroy(_end, this->m_Vector_impl.m_end, this->m_Vector_impl);
                this->m_Vector_impl.m_end -= _end - _start;
                return _end;
            }
            if ( _start <  this->m_Vector_impl.m_end &&
                 _end   >= this->m_Vector_impl.m_end ) {
                std::_Destroy(_start, this->m_Vector_impl.m_end, this->m_Vector_impl);
                this->m_Vector_impl.m_end = _start;
                return this->m_Vector_impl.m_end;
            }
            return this->m_Vector_impl.m_end; }

        iterator
        erase(iterator _elem)
        { return this->erase(_elem, _elem+1); }

        // insert data into the buffer
        //
        template <typename _InputIterator>
        void insert(iterator position, _InputIterator first, _InputIterator last)
        { size_type n = std::distance(first,last);

            if ( position == this->begin() ) {
                if ( n <= this->reverse_capacity() ) {
                    std::copy(first,last, this->begin()-n);
                    this->m_Vector_impl.m_begin -= n;
                    return;
                }     
                size_type sh = n - this->reverse_capacity();
                if (sh > this->capacity()) 
                    std::__throw_out_of_range("buffer::insert");

                std::copy_backward(this->m_Vector_impl.m_begin, this->m_Vector_impl.m_end, 
                                   this->m_Vector_impl.m_end + sh );
                this->m_Vector_impl.m_end += sh;
                std::copy(first,last, this->m_Vector_impl.m_buffer);
                this->m_Vector_impl.m_begin = this->m_Vector_impl.m_buffer;
                return;
            }
            if ( position < this->end()) {
                if ( n <= this->capacity() ) {
                    std::copy_backward(position, this->m_Vector_impl.m_end, this->m_Vector_impl.m_end + n );
                    this->m_Vector_impl.m_end += n;
                    std::copy(first,last, position);
                    return; 
                }
                size_type sh = n - this->capacity();
                if ( sh > this->reverse_capacity())
                    std::__throw_out_of_range("buffer::insert");

                std::copy_backward(position, this->m_Vector_impl.m_end, 
                                   this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size);
                std::copy(this->m_Vector_impl.m_begin, position, this->m_Vector_impl.m_begin-sh);
                this->m_Vector_impl.m_end = this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size;
                this->m_Vector_impl.m_begin -= sh;
                std::copy(first, last, position-sh);
                return;
           }
            if ( position == this->end()) {
                if ( n <= this->capacity() ) {
                    std::copy(first,last, this->end());
                    this->m_Vector_impl.m_end += n;
                    return;
                }
                size_type sh = n - this->capacity();
                if ( sh > this->reverse_capacity() )
                    std::__throw_out_of_range("buffer::insert");
                std::copy(this->m_Vector_impl.m_begin, this->m_Vector_impl.m_end,
                          this->m_Vector_impl.m_begin-sh);
                this->m_Vector_impl.m_begin -= sh; 
                std::copy_backward(first,last, this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size);
                this->m_Vector_impl.m_end = this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size;
                return;
            }
            // unreachable
            assert(!"unreachable!");
        }

        // move data inside the buffer 
        //
        void __shift_begin()
        { if (this->m_Vector_impl.m_begin == this->m_Vector_impl.m_buffer)
                return;

            size_type s = this->size();
            std::copy(this->m_Vector_impl.m_begin, this->m_Vector_impl.m_end, this->m_Vector_impl.m_buffer);

            this->m_Vector_impl.m_begin = this->m_Vector_impl.m_buffer;
            this->m_Vector_impl.m_end = this->m_Vector_impl.m_buffer + s; }
        void __shift_end()
        { if (this->m_Vector_impl.m_end == this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size)
                return;

            size_type s = this->size();
            std::copy_backward(this->m_Vector_impl.m_begin, 
                               this->m_Vector_impl.m_end, this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size );

            this->m_Vector_impl.m_begin = this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size - s;
            this->m_Vector_impl.m_end   = this->m_Vector_impl.m_buffer + this->m_Vector_impl.m_size; }
        void __shift_center()
        { size_type s = this->size();

            if ( this->m_Vector_impl.m_begin == this->m_Vector_impl.m_buffer + ((this->m_Vector_impl.m_size-s)/2) ) {
                return;
            }
            if ( this->m_Vector_impl.m_begin > this->m_Vector_impl.m_buffer + ((this->m_Vector_impl.m_size-s)/2) ) {
                std::copy(this->m_Vector_impl.m_begin, 
                          this->m_Vector_impl.m_end, this->m_Vector_impl.m_buffer + (this->m_Vector_impl.m_size-s)/2 );
            }
            else {
                std::copy_backward(this->m_Vector_impl.m_begin, 
                                   this->m_Vector_impl.m_end, this->m_Vector_impl.m_buffer + (this->m_Vector_impl.m_size-s)/2 + s );
            }

            this->m_Vector_impl.m_begin = this->m_Vector_impl.m_buffer + (this->m_Vector_impl.m_size-s)/2 ;
            this->m_Vector_impl.m_end   = this->m_Vector_impl.m_buffer + (this->m_Vector_impl.m_size-s)/2 + s; }
   };

    // comparisons

    template<class _Tp>
    inline bool operator== (const buffer<_Tp>& x, const buffer<_Tp>& y) 
    { return ( (x.end() - x.begin()) == (y.end()-y.begin()) &&
        std::equal(x.begin(), x.end(), y.begin())); }

    template<class _Tp>
    inline bool operator< (const buffer<_Tp>& x, const buffer<_Tp>& y) 
    { return std::lexicographical_compare(x.begin(),x.end(),y.begin(),y.end()); }

    template<class _Tp>
    inline bool operator!= (const buffer<_Tp>& x, const buffer<_Tp>& y) 
    { return !(x==y); }

    template<class _Tp>
    inline bool operator> (const buffer<_Tp>& x, const buffer<_Tp>& y) 
    { return y<x; }

    template<class _Tp>
    inline bool operator<= (const buffer<_Tp>& x, const buffer<_Tp>& y) 
    { return !(y<x); }

    template<class _Tp>
    inline bool operator>= (const buffer<_Tp>& x, const buffer<_Tp>& y) 
    { return !(x<y); }

    // global swap()
    template<class _Tp>
    inline void swap (buffer<_Tp>& x, buffer<_Tp>& y) 
    { x.swap(y); }

} // namespace more

#endif /* _BUFFER_HH_ */
