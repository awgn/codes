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

namespace more { 

    template<typename Iter> struct const_range_iterator_adapter;

    template<typename Iter>
    struct range_iterator_adapter
    { 
        friend class const_range_iterator_adapter<Iter>;

        typedef std::random_access_iterator_tag                      iterator_category;
        typedef typename std::iterator_traits<Iter>::difference_type difference_type;
        typedef typename std::iterator_traits<Iter>::value_type      value_type;
        typedef typename std::iterator_traits<Iter>::pointer         pointer;
        typedef typename std::iterator_traits<Iter>::reference       reference;

        static_assert(!std::is_const<typename std::remove_pointer<pointer>::type>::value, "const qualified value_type"); 

        template <typename It>
        range_iterator_adapter(It beg, It end)
        : m_begin(beg), m_current(beg), m_end(end)
        {             
            static_assert(
                std::is_same<iterator_category, 
                             typename std::iterator_traits<It>::iterator_category>::value,
                "range_iterator for a non random_access_iterator"); 
        }

        ~range_iterator_adapter()
        {}

        range_iterator_adapter(const range_iterator_adapter& other)
        : m_begin(other.m_begin), m_current(other.m_current), m_end(other.m_end)
        {
            m_range_check();
        }

        range_iterator_adapter& 
        operator=(const range_iterator_adapter& other)  
        {  
            if (this != &other)
            {
                m_begin   = other.m_begin;
                m_current = other.m_current;
                m_end     = other.m_end;
                m_range_check();
            }
            return *this;
        }

        range_iterator_adapter & 
        operator+=(int n)
        { 
            m_current += n;
            return *this;
        }
        friend const range_iterator_adapter operator+(range_iterator_adapter lhs, int n)
        { return lhs+=n; }

        range_iterator_adapter & 
        operator-=(int n)
        { 
            m_current -= n;
            return *this;
        }
        friend const range_iterator_adapter operator-(range_iterator_adapter lhs, int n)
        { return lhs-=n; }

        range_iterator_adapter & 
        operator++()
        { 
            ++m_current;
            return *this;
        }
        range_iterator_adapter  
        operator++(int)
        {
            range_iterator_adapter tmp(*this);
            ++(*this);
            return tmp;
        }

        range_iterator_adapter & 
        operator--()
        { 
            --m_current;
            return *this;
        }
        range_iterator_adapter  
        operator--(int)
        {
            range_iterator_adapter tmp(*this);
            --(*this);
            return tmp;
        }
        
        ssize_t
        size() const
        {
            return std::distance(m_begin, m_current);
        }

        ssize_t
        capacity() const
        {
            return std::distance(m_current, m_end);
        }

        ssize_t
        max_size() const
        {
            return std::distance(m_begin, m_end);
        }

        reference
        operator *()
        {
            m_range_check();
            return * m_current;
        } 
        const reference
        operator *() const
        {
            m_range_check();
            return * m_current;
        } 
 
        pointer
        operator->()
        {
            m_range_check();
            return m_current;
        }
        const pointer 
        operator->() const
        {
            m_range_check();
            return m_current;
        }

    private:
        void m_range_check() const
        {
            if (m_current <  m_begin || m_current >= m_end)
                throw std::runtime_error("range_iterator::m_range_check");
        }
        
        Iter m_begin;
        Iter m_current;
        Iter m_end;
    };
    
    template<typename Iter>
    struct const_range_iterator_adapter
    { 
        typedef std::random_access_iterator_tag                      iterator_category;
        typedef typename std::iterator_traits<Iter>::difference_type difference_type;
        typedef typename std::iterator_traits<Iter>::value_type      value_type;
        typedef typename std::iterator_traits<Iter>::pointer         pointer;
        typedef typename std::iterator_traits<Iter>::reference       reference;

        template <typename It>
        const_range_iterator_adapter(It beg, It end)
        : m_begin(beg), m_current(beg), m_end(end)
        {
            static_assert(
                std::is_same<iterator_category, 
                             typename std::iterator_traits<It>::iterator_category>::value,
                "range_iterator for a non random_access_iterator"); 
        }

        ~const_range_iterator_adapter()
        {}

        const_range_iterator_adapter(const const_range_iterator_adapter& other)
        : m_begin(other.m_begin), m_current(other.m_current), m_end(other.m_end)
        {
            m_range_check();
        }
        explicit const_range_iterator_adapter(const range_iterator_adapter<Iter>& other)
        : m_begin(other.m_begin), m_current(other.m_current), m_end(other.m_end)
        {
            m_range_check();
        }

        const_range_iterator_adapter& 
        operator=(const const_range_iterator_adapter& other)  
        {  
            if (this != &other)
            {
                m_begin   = other.m_begin;
                m_current = other.m_current;
                m_end     = other.m_end;
                m_range_check();
            }
            return *this;
        }
        const_range_iterator_adapter& 
        operator=(const range_iterator_adapter<Iter>& other)  
        {  
            if (this != &other)
            {
                m_begin   = other.m_begin;
                m_current = other.m_current;
                m_end     = other.m_end;
                m_range_check();
            }
            return *this;
        }

        const_range_iterator_adapter & 
        operator+=(int n)
        { 
            m_current += n;
            return *this;
        }
        friend const const_range_iterator_adapter operator+(const_range_iterator_adapter lhs, int n)
        { return lhs+=n; }

        const_range_iterator_adapter & 
        operator-=(int n)
        { 
            m_current -= n;
            return *this;
        }
        friend const const_range_iterator_adapter operator-(const_range_iterator_adapter lhs, int n)
        { return lhs-=n; }

        const_range_iterator_adapter & 
        operator++()
        { 
            ++m_current;
            return *this;
        }
        const_range_iterator_adapter  
        operator++(int)
        {
            const_range_iterator_adapter tmp(*this);
            ++(*this);
            return tmp;
        }

        const_range_iterator_adapter & 
        operator--()
        { 
            --m_current;
            return *this;
        }
        const_range_iterator_adapter  
        operator--(int)
        {
            const_range_iterator_adapter tmp(*this);
            --(*this);
            return tmp;
        }
        
        ssize_t
        size() const
        {
            return std::distance(m_begin, m_current);
        }

        ssize_t
        capacity() const
        {
            return std::distance(m_current, m_end);
        }

        ssize_t
        max_size() const
        {
            return std::distance(m_begin, m_end);
        }

        const reference
        operator *() const
        {
            m_range_check();
            return * m_current;
        } 
 
        const pointer 
        operator->() const
        {
            m_range_check();
            return m_current;
        }

    private:
        void m_range_check() const
        {
            if (m_current <  m_begin || m_current >= m_end)
                throw std::runtime_error("const_range_iterator::m_range_check");
        }
        
        Iter m_begin;
        Iter m_current;
        Iter m_end;
    };

    template <typename Iter>
    range_iterator_adapter<Iter>
    range_iterator(Iter start, Iter stop)
    {
        return range_iterator_adapter<Iter>(start,stop);
    }

    template <typename Iter>
    const_range_iterator_adapter<Iter>
    const_range_iterator(Iter start, Iter stop)
    {
        return const_range_iterator_adapter<Iter>(start,stop);
    }

} // namespace more

#endif /* _RANGE_ITERATOR_HPP_ */
