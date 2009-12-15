/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _CURSOR_HH_
#define _CURSOR_HH_ 

#include <stdexcept>

namespace more { 

    template <typename T>
    class cursor
    {
        template <typename P>
        void boundary_alignment_check(P *start, P *end)
        {
             if ( (end-start) % sizeof(T) )
                throw std::runtime_error("cursor::cursor: bad boundaries");
        }

        void range_check()
        {
            if (_M_cur <  _M_beg || _M_cur >= _M_end)
                throw std::runtime_error("cursor::range_check");
        }

    public:
        typedef T           value_type;
        typedef T *         iterator;
        typedef const T *   const_iterator;

        template <typename P>
        cursor(P *beg, P *end)
        : _M_beg(reinterpret_cast<T *>(beg)),
          _M_end(reinterpret_cast<T *>(end)),
          _M_cur(_M_beg) 
        {
            boundary_alignment_check(beg,end);
        }

        ~cursor()
        {}

        cursor(T *beg, T *cur, T *end)
        : _M_beg(beg),
          _M_cur(cur),
          _M_end(end)
        {
            range_check();
        }

        cursor(cursor &rhs)
        : _M_beg(rhs._M_beg),
          _M_end(rhs._M_end),
          _M_cur(rhs._M_cur)
        {
            range_check();
        }

        cursor & 
        operator=(const cursor &rhs)  
        {  
            _M_beg = rhs._M_beg;
            _M_end = rhs._M_end;
            _M_cur = rhs._M_cur;

            range_check();
            return *this;
        }

        template <typename P>
        cursor(const cursor<P> &rhs)
        : _M_beg(reinterpret_cast<T *>(rhs._M_beg)),
          _M_end(reinterpret_cast<T *>(rhs._M_end)),
          _M_cur(reinterpret_cast<T *>(rhs._M_cur))
        {
            boundary_alignment_check(rhs._M_beg,rhs._M_end);
            range_check();
        }

        template <typename P>
        cursor & operator=(const cursor<P> &rhs)  
        {  
            boundary_alignment_check(rhs._M_beg,rhs._M_end);

            _M_beg = reinterpret_cast<T *>(rhs._M_beg);
            _M_end = reinterpret_cast<T *>(rhs._M_end);
            _M_cur = reinterpret_cast<T *>(rhs._M_cur);
            
            range_check();
            return *this;
        }
    
        ////////////////////////////////////////////////
        // artimentic...

        cursor & 
        operator+=(int n)
        { 
            _M_cur += n;
            return *this;
        }
        friend const cursor operator+(cursor lhs, int n)
        { return lhs+=n; }

        cursor & 
        operator-=(int n)
        { 
            _M_cur -= n;
            return *this;
        }
        friend const cursor operator-(cursor lhs, int n)
        { return lhs-=n; }


        cursor & 
        operator++()
        { 
            ++_M_cur;
            return *this;
        }
        cursor  
        operator++(int)
        {
            cursor tmp(*this);
            ++(*this);
            return tmp;
        }

        cursor & 
        operator--()
        { 
            --_M_cur;
            return *this;
        }
        cursor  
        operator--(int)
        {
            cursor tmp(*this);
            --(*this);
            return tmp;
        }

        ////////////////////////////////////////////////
        // readers...

        ssize_t
        size() const
        {
            return _M_end-_M_cur;
        }

        iterator 
        begin() 
        {
            return _M_beg;
        }
        
        const_iterator
        begin() const
        { 
            return _M_beg;
        }

        iterator
        end() 
        {
            return _M_end;
        }

        const_iterator
        end() const 
        {
            return _M_end;
        }

        iterator 
        cur() 
        { 
            return _M_cur;
        }

        const_iterator 
        cur() const
        { 
            return _M_cur;
        }

        ////////////////////////////////////////////////
        // implicit conversions

        operator T *()
        {
            return _M_cur;
        }

        T &
        operator *()
        {
            range_check();
            return * _M_cur;
        } 

        T *
        operator->()
        {
            range_check();
            return _M_cur;
        }

    private:
        T * _M_beg;
        T * _M_end;
        T * _M_cur;

    };
} // namespace more

#endif /* _CURSOR_HH_ */
