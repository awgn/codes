/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _RANGE_ITERATOR_HPP_
#define _RANGE_ITERATOR_HPP_ 

#include <iterator>
#include <type_traits>
#include <stdexcept>
#include <cassert>
#include <iostream>

#include <typeinfo>


namespace more { 

    template<typename Iter> struct range_const_iterator_adapter;

    template <typename Iter>
    struct range_iterator_base
    {
        range_iterator_base(Iter beg, Iter cur, Iter end, bool out_of_range)
        : m_begin(beg), m_current(cur), m_end(end), m_out_of_range(out_of_range)
        {}

        range_iterator_base()
        : m_begin(), m_current(), m_end(), m_out_of_range(true)
        {}

        ~range_iterator_base()
        {}

        void m_range_check(std::random_access_iterator_tag) const
        {
            if (m_current <  m_begin || (m_current+1) > m_end)
                throw std::runtime_error("random_access_iterator::m_range_check");
        }    
        void m_range_check(std::bidirectional_iterator_tag) const
        {
            if (m_out_of_range || m_current == m_end)
                throw std::runtime_error("bidirectional_iterator::m_range_check");
        }

        void m_range_check(std::forward_iterator_tag) const
        {
            if (m_out_of_range || m_current == m_end)
                throw std::runtime_error("forward_iterator::m_range_check");
        }
    
        void m_track(std::random_access_iterator_tag, bool) const
        { /* nothing to do */ }

        void m_track(std::bidirectional_iterator_tag, bool direction) const
        {
            if (direction)
            {
                if (!m_out_of_range &&
                    m_current == m_end) {
                    m_out_of_range = true;
                    return;
                }
                if (m_out_of_range &&
                    m_current == m_begin) {
                    m_out_of_range = false;
                    return;
                }
            }
            else
            {
                if (!m_out_of_range &&
                    m_current == std::advance(m_begin, -1)) {
                    m_out_of_range = true;
                    return;
                }
                if (m_out_of_range &&
                    m_current == std::advance(m_end, -1)) {
                    m_out_of_range = false;
                    return;
                }
            }
        }
        void m_track(std::forward_iterator_tag, bool) const
        {
            if (!m_out_of_range &&
                m_current == m_end) {
                m_out_of_range = true;
                return;
            }
            if (m_out_of_range &&
                m_current == m_begin) {
                m_out_of_range = false;
                return;
            }
        }

        Iter m_begin;
        Iter m_current;
        Iter m_end;
        
        mutable bool m_out_of_range;
    };


    template<typename Iter>
    struct range_iterator_adapter : protected range_iterator_base<Iter>
    { 
        friend struct range_const_iterator_adapter<Iter>;

        typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;
        typedef typename std::iterator_traits<Iter>::difference_type   difference_type;
        typedef typename std::iterator_traits<Iter>::value_type        value_type;
        typedef typename std::iterator_traits<Iter>::pointer           pointer;
        typedef typename std::iterator_traits<Iter>::reference         reference;

        static_assert(!std::is_const<typename std::remove_pointer<pointer>::type>::value, "range_iterator for a const_iterator type"); 

        template <typename It>
        range_iterator_adapter(It beg, It end)
        : range_iterator_base<Iter>(beg,beg,end,false)
        {}

        range_iterator_adapter()
        : range_iterator_base<Iter>()
        {}

        ~range_iterator_adapter()
        {}

        range_iterator_adapter(const range_iterator_adapter& other)
        : range_iterator_base<Iter>(other.m_begin, other.m_current, other.m_end, other.m_out_of_range)
        {}
        
    private:
        template <typename It>
        range_iterator_adapter(It beg, It cur, It end, bool out)
        : range_iterator_base<Iter>(beg, cur, end, out)
        {}

    public:
        // forward iterator requirements
        //

        reference
        operator *() const
        {
            this->m_range_check(iterator_category());
            return * this->m_current;
        } 
 
        pointer 
        operator->() const
        {
            this->m_range_check(iterator_category());
            return this->m_current;
        }

        range_iterator_adapter & 
        operator++()
        {   
            ++this->m_current;
            this->m_track(iterator_category(),true);
            return *this;
        }
        range_iterator_adapter  
        operator++(int)
        {
            range_iterator_adapter tmp(*this);
            ++(*this);
            this->m_track(iterator_category(),true);
            return tmp;
        }

        friend bool operator==(const range_iterator_adapter &lhs, const range_iterator_adapter &rhs) 
        {
            assert(lhs.m_begin == rhs.m_begin); assert(lhs.m_end == rhs.m_end);
            return lhs.m_current == rhs.m_current; 
        }

        friend bool operator!=(const range_iterator_adapter &lhs, const range_iterator_adapter &rhs)
        {
            return !(lhs == rhs);
        }

        range_iterator_adapter& 
        operator=(const range_iterator_adapter& other)  
        {  
            if (this != &other)
            {
                this->m_begin   = other.m_begin;
                this->m_current = other.m_current;
                this->m_end     = other.m_end;
            }
            return *this;
        }

        // bidirectional iterator requirements
        //

        range_iterator_adapter & 
        operator--()
        { 
            --this->m_current;
            this->m_track(iterator_category(),false);
            return *this;
        }
        range_iterator_adapter  
        operator--(int)
        {
            range_iterator_adapter tmp(*this);
            --(*this);
            this->m_track(iterator_category(),false);
            return tmp;
        }

        // random access iterator requirements
        //

        reference
        operator[](const difference_type &n) const
        {
            return this->m_current[n];
        }

        range_iterator_adapter & 
        operator+=(ptrdiff_t n)
        { 
            this->m_current += n;
            return *this;
        }
        friend const range_iterator_adapter operator+(range_iterator_adapter lhs, int n)
        { return lhs+=n; }

        range_iterator_adapter & 
        operator-=(ptrdiff_t n)
        { 
            this->m_current -= n;
            return *this;
        }
        friend const range_iterator_adapter operator-(range_iterator_adapter lhs, int n)
        { return lhs-=n; }
        
        friend difference_type operator-(const range_iterator_adapter& lhs,
                                         const range_iterator_adapter& rhs)
        {
            assert(lhs.m_begin == rhs.m_begin); assert(lhs.m_end == rhs.m_end);
            return lhs.m_current-rhs.m_current;
        } 

        friend bool operator<(const range_iterator_adapter& lhs,
                              const range_iterator_adapter& rhs)
        {
            assert(lhs.m_begin == rhs.m_begin); assert(lhs.m_end == rhs.m_end);
            return lhs.m_current < rhs.m_current;
        }
        friend bool operator> (const range_iterator_adapter& lhs,
                               const range_iterator_adapter& rhs)
        {
            return rhs < lhs;
        }
        friend bool operator>=(const range_iterator_adapter& lhs,
                               const range_iterator_adapter& rhs)
        {
            return !(lhs<rhs);
        }
        friend bool operator<=(const range_iterator_adapter& lhs,
                               const range_iterator_adapter& rhs)
        {
            return !(lhs>rhs);
        }

        range_iterator_adapter
        begin() const
        {
            return range_iterator_adapter(this->m_begin, this->m_begin, this->m_end, this->m_out_of_range);
        }

        range_iterator_adapter
        end() const
        {
            return range_iterator_adapter(this->m_begin, this->m_end, this->m_end, this->m_out_of_range);
        }
        
        ssize_t
        size() const
        {
            return std::distance(this->m_begin, this->m_end);
        }

        ssize_t
        capacity() const
        {
            return std::distance(this->m_current, this->m_end);
        }
    };

    template<typename Iter>
    struct range_const_iterator_adapter : protected range_iterator_base<Iter>
    { 
        typedef typename std::iterator_traits<Iter>::iterator_category iterator_category;
        typedef typename std::iterator_traits<Iter>::difference_type   difference_type;
        typedef typename std::iterator_traits<Iter>::value_type        value_type;
        typedef typename std::iterator_traits<Iter>::pointer           pointer;
        typedef typename std::iterator_traits<Iter>::reference         reference;

        template <typename It>
        range_const_iterator_adapter(It beg, It end)
        : range_iterator_base<Iter>(beg,beg,end,false)
        {}

        range_const_iterator_adapter()
        : range_iterator_base<Iter>()
        {}

        ~range_const_iterator_adapter()
        {}

        range_const_iterator_adapter(const range_const_iterator_adapter& other)
        : range_iterator_base<Iter>(other.m_begin, other.m_current, other.m_end, other.m_out_of_range)
        {}
        
        /* note: this must be template otherwise it has greater precedence over 
                 the first template constructor
          */ 
        template <typename It>
        range_const_iterator_adapter(const range_iterator_adapter<It>& other)
        : range_iterator_base<Iter>(other.m_begin, other.m_current, other.m_end, other.m_out_of_range)
        {}

    private:
        template <typename It>
        range_const_iterator_adapter(It beg, It cur, It end, bool out)
        : range_iterator_base<Iter>(beg, cur, end, out)
        {}

    public:
        // forward iterator requirements
        //

        const reference
        operator *() const
        {
            this->m_range_check(iterator_category());
            return * this->m_current;
        } 
 
        const pointer 
        operator->() const
        {
            this->m_range_check(iterator_category());
            return this->m_current;
        }

        range_const_iterator_adapter & 
        operator++()
        {   
            ++this->m_current;
            this->m_track(iterator_category(),true);
            return *this;
        }
        range_const_iterator_adapter  
        operator++(int)
        {
            range_const_iterator_adapter tmp(*this);
            ++(*this);
            this->m_track(iterator_category(),true);
            return tmp;
        }

        friend bool operator==(const range_const_iterator_adapter &lhs, const range_const_iterator_adapter &rhs) 
        {
            assert(lhs.m_begin == rhs.m_begin); assert(lhs.m_end == rhs.m_end);
            return lhs.m_current == rhs.m_current; 
        }

        friend bool operator!=(const range_const_iterator_adapter &lhs, const range_const_iterator_adapter &rhs)
        {
            return !(lhs == rhs);
        }

        range_const_iterator_adapter& 
        operator=(const range_const_iterator_adapter& other)  
        {  
            if (this != &other)
            {
                this->m_begin   = other.m_begin;
                this->m_current = other.m_current;
                this->m_end     = other.m_end;
            }
            return *this;
        }

        // bidirectional iterator requirements
        //

        range_const_iterator_adapter & 
        operator--()
        { 
            --this->m_current;
            this->m_track(iterator_category(),false);
            return *this;
        }
        range_const_iterator_adapter  
        operator--(int)
        {
            range_const_iterator_adapter tmp(*this);
            --(*this);
            this->m_track(iterator_category(),false);
            return tmp;
        }

        // random access iterator requirements
        //

        reference
        operator[](const difference_type &n) const
        {
            return this->m_current[n];
        }

        range_const_iterator_adapter & 
        operator+=(ptrdiff_t n)
        { 
            this->m_current += n;
            return *this;
        }
        friend const range_const_iterator_adapter operator+(range_const_iterator_adapter lhs, int n)
        { return lhs+=n; }

        range_const_iterator_adapter & 
        operator-=(ptrdiff_t n)
        { 
            this->m_current -= n;
            return *this;
        }
        friend const range_const_iterator_adapter operator-(range_const_iterator_adapter lhs, int n)
        { return lhs-=n; }
        
        friend difference_type operator-(const range_const_iterator_adapter& lhs,
                                         const range_const_iterator_adapter& rhs)
        {
            assert(lhs.m_begin == rhs.m_begin); assert(lhs.m_end == rhs.m_end);
            return lhs.m_current-rhs.m_current;
        } 

        friend bool operator<(const range_const_iterator_adapter& lhs,
                              const range_const_iterator_adapter& rhs)
        {
            assert(lhs.m_begin == rhs.m_begin); assert(lhs.m_end == rhs.m_end);
            return lhs.m_current < rhs.m_current;
        }
        friend bool operator> (const range_const_iterator_adapter& lhs,
                               const range_const_iterator_adapter& rhs)
        {
            return rhs < lhs;
        }
        friend bool operator>=(const range_const_iterator_adapter& lhs,
                               const range_const_iterator_adapter& rhs)
        {
            return !(lhs<rhs);
        }
        friend bool operator<=(const range_const_iterator_adapter& lhs,
                               const range_const_iterator_adapter& rhs)
        {
            return !(lhs>rhs);
        }

        range_const_iterator_adapter
        begin() const
        {
            return range_const_iterator_adapter(this->m_begin, this->m_begin, this->m_end, this->m_out_of_range);
        }

        range_const_iterator_adapter
        end() const
        {
            return range_const_iterator_adapter(this->m_begin, this->m_end, this->m_end, this->m_out_of_range);
        }
        
        ssize_t
        size() const
        {
            return std::distance(this->m_begin, this->m_end);
        }

        ssize_t
        capacity() const
        {
            return std::distance(this->m_current, this->m_end);
        }
    };

    template <typename Iter>
    range_iterator_adapter<Iter>
    range_iterator(Iter start, Iter end)
    {
        return range_iterator_adapter<Iter>(start, end);
    }

    template <typename Iter>
    range_const_iterator_adapter<Iter>
    range_const_iterator(Iter start, Iter end)
    {
        return (range_const_iterator_adapter<Iter>(start, end));
    }

} // namespace more

#endif /* _RANGE_ITERATOR_HPP_ */
