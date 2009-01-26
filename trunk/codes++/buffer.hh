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
            _Tp *       _M_buffer;
            std::size_t _M_size;
            _Tp *       _M_begin;
            _Tp *       _M_end;
            _Vector_impl(_Tp_alloc_type const& __a) 
            : _Tp_alloc_type(__a), _M_buffer(0), _M_size(0), _M_begin(0), _M_end(0)
            {}

        } _M_Vector_impl;

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

        buffer(size_type __n = 4096, const value_type& __value = value_type(), const allocator_type& __a = allocator_type())
        : _M_Vector_impl(__a)
        {
            this->_M_Vector_impl._M_buffer = this->_M_Vector_impl.allocate(__n);
            this->_M_Vector_impl._M_size = __n;

            std::__uninitialized_fill_n_a(this->_M_Vector_impl._M_buffer, __n, __value, const_cast<allocator_type &>(__a));
            _M_Vector_impl._M_begin = _M_Vector_impl._M_end = this->_M_Vector_impl._M_buffer; }

        ~buffer()
        {  std::_Destroy(this->_M_Vector_impl._M_buffer, 
                         this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size, this->_M_Vector_impl); }

        // assignment and copy constructor
        //
        buffer(const buffer & rhs)
        : _M_Vector_impl(rhs._M_Vector_impl)
        {
            _M_Vector_impl._M_buffer = _M_Vector_impl._M_begin = _M_Vector_impl._M_end = _M_Vector_impl.allocate(rhs.max_size());
            _M_Vector_impl._M_size = rhs.max_size();
            std::__uninitialized_copy_a(rhs.begin(), rhs.end(), this->_M_Vector_impl._M_begin, 
                                        const_cast<typename buffer<_Tp,_Alloc>::_Vector_impl &>(rhs._M_Vector_impl));
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
        { return this->_M_Vector_impl._M_begin; }

        const_iterator 
        begin() const 
        { return this->_M_Vector_impl._M_begin; }

        iterator 
        end() 
        { return this->_M_Vector_impl._M_end; }

        const_iterator 
        end() const 
        { return this->_M_Vector_impl._M_end; }

        reverse_iterator
        rbegin()
        { return reverse_iterator(end()); }

        const_reverse_iterator
        rbegin() const
        { return const_reverse_iterator(end()); }

        reverse_iterator
        rend()
        { return reverse_iterator(begin()); }

        const_reverse_iterator
        rend() const
        { return const_reverse_iterator(begin()); }

        // return the number of elements in the buffer
        //
        size_type size() const 
        { return this->_M_Vector_impl._M_end - this->_M_Vector_impl._M_begin; }

        // return the max. size available for appending data
        //
        size_type capacity() const
        { return this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size - this->_M_Vector_impl._M_end; } 

        // return the max. size available for inserting data 
        //
        size_type reverse_capacity() const
        { return this->_M_Vector_impl._M_begin - this->_M_Vector_impl._M_buffer; } 

        // return the max. size of buffer
        size_type max_size() const
        { return this->_M_Vector_impl._M_size; }

        // commit n-elements after a raw-write... 
        //
        void commit(size_type n)
        { this->_M_Vector_impl._M_end += std::min(this->capacity(), n); }

        // flush data of buffer...
        //
        void clear()
        {   // optimized away for trivial descrutors
            std::_Destroy(this->_M_Vector_impl._M_begin, this->_M_Vector_impl._M_end, this->_M_Vector_impl);
            this->_M_Vector_impl._M_end = this->_M_Vector_impl._M_begin; }

        // discard n-elements from begin()
        //
        void discard(size_type n)
        { n = std::min(this->size(),n);
            // optimized away for trivial descrutors
            std::_Destroy(this->_M_Vector_impl._M_begin, this->_M_Vector_impl._M_begin + n, this->_M_Vector_impl);
            this->_M_Vector_impl._M_begin += n; }

        // flush the whole buffer, reset begin() and end()...
        //
        void reset()
        { std::_Destroy(this->_M_Vector_impl._M_begin, this->_M_Vector_impl._M_begin + this->_M_Vector_impl._M_size, this->_M_Vector_impl);
            this->_M_Vector_impl._M_begin = this->_M_Vector_impl._M_buffer; 
            this->_M_Vector_impl._M_end = this->_M_Vector_impl._M_buffer; }

        // return true if buffer is empty
        //
        bool empty() const
        { return !(this->_M_Vector_impl._M_end-this->_M_Vector_impl._M_begin); }

        // operator[]: element access
        //
        reference
        operator[](size_type i)
        { return this->_M_Vector_impl._M_begin[i]; }

        const_reference
        operator[](size_type i) const
        { return this->_M_Vector_impl._M_begin[i]; }

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

        // direct access to data, for reading and writing 
        //
        const _Tp* 
        data_read() const 
        { return this->_M_Vector_impl._M_begin; }

        _Tp* 
        data_write() 
        { return this->_M_Vector_impl._M_end; }

        // swap 
        //
        void 
        swap (buffer& rhs) 
        { std::swap(this->_M_Vector_impl._M_buffer, rhs._M_Vector_impl._M_buffer);
            std::swap(this->_M_Vector_impl._M_size, rhs._M_Vector_impl._M_size);
            std::swap(this->_M_Vector_impl._M_begin, rhs._M_Vector_impl._M_begin);
            std::swap(this->_M_Vector_impl._M_end, rhs._M_Vector_impl._M_end); }
 
        // push/pop
        //
        bool push_back(const _Tp &e)
        { if ( !(this->_M_Vector_impl._M_end < this->_M_Vector_impl._M_buffer+this->_M_Vector_impl._M_size) ) {
                return false;
            }
            *(this->_M_Vector_impl._M_end++) = e; 
            return true; }

        bool pop_back()
        { if ( !(this->_M_Vector_impl._M_end > this->_M_Vector_impl._M_begin) ) {
                return false;
            }
            --this->_M_Vector_impl._M_end;
            this->_M_Vector_impl.destroy(this->_M_Vector_impl._M_end);
            return true; }

        bool push_front(const _Tp &e)
        { if ( !(this->_M_Vector_impl._M_begin > this->_M_Vector_impl._M_buffer) ) {
                return false;
            }
            *(--this->_M_Vector_impl._M_begin) = e;
            return true; }

        bool pop_front()
        { if ( !(this->_M_Vector_impl._M_begin < this->_M_Vector_impl._M_end) ) {
                return false; 
            }
            this->_M_Vector_impl.destroy(this->_M_Vector_impl._M_begin++);
            return true; }   

        // erase data from buffer...
        //
        iterator
        erase(iterator _start, iterator _end)
        { if ( _start <  this->_M_Vector_impl._M_begin &&
                 _end   <= this->_M_Vector_impl._M_begin ) {
                return this->_M_Vector_impl._M_begin;
            }
            if ( _start <= this->_M_Vector_impl._M_begin &&
                 _end   <= this->_M_Vector_impl._M_end ) {
                std::_Destroy(this->_M_Vector_impl._M_begin, _end, this->_M_Vector_impl);
                this->_M_Vector_impl._M_begin = _end;
                return this->_M_Vector_impl._M_begin;
            }
            if ( _start <  this->_M_Vector_impl._M_end &&
                 _end   <  this->_M_Vector_impl._M_end ) {
                std::copy(_end, this->_M_Vector_impl._M_end, _start);
                std::_Destroy(_end, this->_M_Vector_impl._M_end, this->_M_Vector_impl);
                this->_M_Vector_impl._M_end -= _end - _start;
                return _end;
            }
            if ( _start <  this->_M_Vector_impl._M_end &&
                 _end   >= this->_M_Vector_impl._M_end ) {
                std::_Destroy(_start, this->_M_Vector_impl._M_end, this->_M_Vector_impl);
                this->_M_Vector_impl._M_end = _start;
                return this->_M_Vector_impl._M_end;
            }
            return this->_M_Vector_impl._M_end; }

        iterator
        erase(iterator _elem)
        { return this->erase(_elem, _elem+1); }

        // insert data into the buffer
        //
        template <typename _InputIterator>
        bool insert(iterator position, _InputIterator first, _InputIterator last)
        { size_type n = std::distance(first,last);

            if ( position == this->begin() ) {
                if ( n <= this->reverse_capacity() ) {
                    std::copy(first,last, this->begin()-n);
                    this->_M_Vector_impl._M_begin -= n;
                    return true;
                }     
                size_type sh = n - this->reverse_capacity();
                if (sh > this->capacity()) 
                    return false; 
                std::copy_backward(this->_M_Vector_impl._M_begin, this->_M_Vector_impl._M_end, 
                                   this->_M_Vector_impl._M_end + sh );
                this->_M_Vector_impl._M_end += sh;
                std::copy(first,last, this->_M_Vector_impl._M_buffer);
                this->_M_Vector_impl._M_begin = this->_M_Vector_impl._M_buffer;
                return true;

            }
            if ( position < this->end()) {
                if ( n <= this->capacity() ) {
                    std::copy_backward(position, this->_M_Vector_impl._M_end, this->_M_Vector_impl._M_end + n );
                    this->_M_Vector_impl._M_end += n;
                    std::copy(first,last, position);
                    return true; 
                }
                size_type sh = n - this->capacity();
                if ( sh > this->reverse_capacity())
                    return false;

                std::copy_backward(position, this->_M_Vector_impl._M_end, 
                                   this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size);
                std::copy(this->_M_Vector_impl._M_begin, position, this->_M_Vector_impl._M_begin-sh);
                this->_M_Vector_impl._M_end = this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size;
                this->_M_Vector_impl._M_begin -= sh;
                std::copy(first, last, position-sh);
                return true;
           }
            if ( position == this->end()) {
                if ( n <= this->capacity() ) {
                    std::copy(first,last, this->end());
                    this->_M_Vector_impl._M_end += n;
                    return true;
                }
                size_type sh = n - this->capacity();
                if ( sh > this->reverse_capacity() )
                    return false;
                std::copy(this->_M_Vector_impl._M_begin, this->_M_Vector_impl._M_end,
                          this->_M_Vector_impl._M_begin-sh);
                this->_M_Vector_impl._M_begin -= sh; 
                std::copy_backward(first,last, this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size);
                this->_M_Vector_impl._M_end = this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size;
                return true;
            }
            // unreachable
            return false;
        }

        // move data inside the buffer 
        //
        void __shift_begin()
        { if (this->_M_Vector_impl._M_begin == this->_M_Vector_impl._M_buffer)
                return;

            size_type s = this->size();
            std::copy(this->_M_Vector_impl._M_begin, this->_M_Vector_impl._M_end, this->_M_Vector_impl._M_buffer);

            this->_M_Vector_impl._M_begin = this->_M_Vector_impl._M_buffer;
            this->_M_Vector_impl._M_end = this->_M_Vector_impl._M_buffer + s; }
        void __shift_end()
        { if (this->_M_Vector_impl._M_end == this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size)
                return;

            size_type s = this->size();
            std::copy_backward(this->_M_Vector_impl._M_begin, 
                               this->_M_Vector_impl._M_end, this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size );

            this->_M_Vector_impl._M_begin = this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size - s;
            this->_M_Vector_impl._M_end   = this->_M_Vector_impl._M_buffer + this->_M_Vector_impl._M_size; }
        void __shift_center()
        { size_type s = this->size();

            if ( this->_M_Vector_impl._M_begin == this->_M_Vector_impl._M_buffer + ((this->_M_Vector_impl._M_size-s)/2) ) {
                return;
            }
            if ( this->_M_Vector_impl._M_begin > this->_M_Vector_impl._M_buffer + ((this->_M_Vector_impl._M_size-s)/2) ) {
                std::copy(this->_M_Vector_impl._M_begin, 
                          this->_M_Vector_impl._M_end, this->_M_Vector_impl._M_buffer + (this->_M_Vector_impl._M_size-s)/2 );
            }
            else {
                std::copy_backward(this->_M_Vector_impl._M_begin, 
                                   this->_M_Vector_impl._M_end, this->_M_Vector_impl._M_buffer + (this->_M_Vector_impl._M_size-s)/2 + s );
            }

            this->_M_Vector_impl._M_begin = this->_M_Vector_impl._M_buffer + (this->_M_Vector_impl._M_size-s)/2 ;
            this->_M_Vector_impl._M_end   = this->_M_Vector_impl._M_buffer + (this->_M_Vector_impl._M_size-s)/2 + s; }
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
