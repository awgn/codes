/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_CURSOR_HPP_
#define _MORE_CURSOR_HPP_ 

#include <type_traits>   
#include <stdexcept>

namespace more { 

    template <typename T>
    class cursor
    {
        template <typename P>
        void m_boundary_alignment_check(P *start, P *end)
        {
             if ( (std::distance(start,end) * sizeof(P)) % sizeof(T) )
                throw std::runtime_error("cursor::m_boundary_alignment_check");
        }

        void m_range_check()
        {
            if (m_cur <  m_beg || m_cur >= m_end)
                throw std::runtime_error("cursor::m_range_check");
        }

    public:
        typedef std::random_access_iterator_tag  iterator_category;
        typedef T                                value_type;
        typedef std::ptrdiff_t                   difference_type;
        typedef T *                              pointer;
        typedef T &                              reference;

        typedef T *                              iterator;
        typedef const T *                        const_iterator;

        friend class cursor< typename std::add_const<T>::type >;

        template <typename P>
        cursor(P *beg, P *end)
        : m_beg(reinterpret_cast<T *>(beg)),
          m_end(reinterpret_cast<T *>(end)),
          m_cur(m_beg) 
        {
            m_boundary_alignment_check(beg,end);
        }

        ~cursor()
        {}

        cursor(T *beg, T *cur, T *end)
        : m_beg(beg),
          m_cur(cur),
          m_end(end)
        {
            m_range_check();
        }

        cursor(cursor &rhs)
        : m_beg(rhs.m_beg),
          m_end(rhs.m_end),
          m_cur(rhs.m_cur)
        {
            m_range_check();
        }

        cursor & 
        operator=(const cursor &rhs)  
        {  
            if (this != &rhs)
            {
                m_beg = rhs.m_beg;
                m_end = rhs.m_end;
                m_cur = rhs.m_cur;
                m_range_check();
            }
            return *this;
        }

        template <typename P>
        cursor(const cursor<P> &rhs)
        : m_beg(reinterpret_cast<T *>(rhs.m_beg)),
          m_end(reinterpret_cast<T *>(rhs.m_end)),
          m_cur(reinterpret_cast<T *>(rhs.m_cur))
        {
            m_boundary_alignment_check(rhs.m_beg,rhs.m_end);
            m_range_check();
        }

        template <typename P>
        cursor & operator=(const cursor<P> &rhs)  
        {  
            if (this != &rhs)
            {
                boundary_alignment_check(rhs.m_beg,rhs.m_end);
                m_beg = reinterpret_cast<T *>(rhs.m_beg);
                m_end = reinterpret_cast<T *>(rhs.m_end);
                m_cur = reinterpret_cast<T *>(rhs.m_cur);
                m_range_check();
            }
            return *this;
        }
    
        ////////////////////////////////////////////////

        cursor & 
        operator+=(int n)
        { 
            m_cur += n;
            return *this;
        }
        friend const cursor operator+(cursor lhs, int n)
        { return lhs+=n; }

        cursor & 
        operator-=(int n)
        { 
            m_cur -= n;
            return *this;
        }
        friend const cursor operator-(cursor lhs, int n)
        { return lhs-=n; }

        cursor & 
        operator++()
        { 
            ++m_cur;
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
            --m_cur;
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

        // ssize_t
        // size() const
        // {
        //     return std::distance(m_beg, m_cur);
        // }

        ssize_t
        capacity() const
        {
            return std::distance(m_cur, m_end);
        }

        ssize_t
        max_size() const
        {
            return std::distance(m_beg, m_end);
        }
 
        ////////////////////////////////////////////////
        // iterators...
        
        iterator 
        begin() 
        {
            return m_beg;
        }
        
        const_iterator
        begin() const
        { 
            return m_beg;
        }

        const_iterator
        cbegin() const
        { 
            return m_beg;
        }

        iterator
        end() 
        {
            return m_end;
        }

        const_iterator
        end() const 
        {
            return m_end;
        }

        const_iterator
        cend() const 
        {
            return m_end;
        }

        iterator 
        cur() 
        { 
            return m_cur;
        }

        const_iterator 
        cur() const
        { 
            return m_cur;
        }

        ////////////////////////////////////////////////
        // operators:

        T &
        operator *()
        {
            m_range_check();
            return * m_cur;
        } 

        const T &
        operator *() const
        {
            m_range_check();
            return * m_cur;
        } 
 
        T *
        operator->()
        {
            m_range_check();
            return m_cur;
        }

        const T *
        operator->() const
        {
            m_range_check();
            return m_cur;
        }
 
    private:
        T * m_beg;
        T * m_end;
        T * m_cur;
    };

} // namespace more

#endif /* _MORE_CURSOR_HPP_ */
