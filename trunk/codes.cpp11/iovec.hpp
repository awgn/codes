/* $Id: iovec-utils.hh 500 2010-03-28 12:15:09Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_IOVEC_HPP_
#define _MORE_IOVEC_HPP_ 

#include <vector>
#include <iterator>
#include <iostream>
#include <type_traits>
#include <cassert>
#include <stdexcept>

#include <sys/uio.h>

namespace more 
{
    namespace iovec_detail {

        template <typename Tp>
        typename std::remove_const<Tp>::type *
        drop_const(Tp * ptr)
        {
            return const_cast<typename std::remove_const<Tp>::type *>(ptr);
        }

        struct address_of 
        {
            template <typename T>
            static typename std::enable_if<std::is_integral<T>::value,
                            typename std::add_const<T>::type *>::type 
            value(const T &elem)
            {
                return &elem;
            }

            static const char *
            value(const std::string & elem)
            {
                return elem.c_str();
            }
        };

        template <typename Iter>
        struct size_of 
        {
            template <typename T>
            static typename std::enable_if<std::is_integral<T>::value, size_t>::type 
            value(T)
            {
                return sizeof(T);
            }

            static
            size_t value(const std::string &s)
            {
                return s.size();
            }
        };

        // get_iovec for forward iterator:
        //

        template <typename Iterator>
        std::vector<iovec>
        get_iovec(Iterator __it, Iterator __end, std::forward_iterator_tag)
        {
            std::vector<iovec> ret;

            if ( __it == __end )
                return ret;

            auto base = address_of::value(*__it); 
            size_t len = 0;

            for(auto ptr = base; __it != __end;  len += size_of<Iterator>::value(*__it++), ++ptr )
            {
                if ( ptr != address_of::value(*__it))
                {
                    iovec iov = { static_cast<void *>(drop_const(base)), len };
                    ret.push_back(iov);
                    base = ptr = address_of::value(*__it);
                    len  = 0;
                }
            }

            iovec iov = { static_cast<void *>(drop_const(base)), len };
            ret.push_back(iov);
            return ret;
        }    

        template <typename Iterator>
        typename std::enable_if<
            std::is_integral<typename std::iterator_traits<Iterator>::value_type>::value,
                std::vector<iovec> >::type
        get_iovec(Iterator __it, Iterator __end, std::random_access_iterator_tag)
        {
            auto n = std::distance(__it,__end);

            if ( n && n == (&*__end - &*__it) ) 
            {
                iovec iov = { static_cast<void *>(& *__it), 
                    static_cast<size_t>(n) * sizeof(typename std::iterator_traits<Iterator>::value_type) };
                return std::vector<iovec>(1, iov);
            }

            return get_iovec(__it,__end, std::forward_iterator_tag());
        }
    }

    // get_iovec algorithm: given a generic range, return a std::vector<iovec> descriptor!
    //

    template <typename Iterator>
    std::vector<iovec>
    get_iovec(Iterator __it, Iterator __end)
    {
        return iovec_detail::get_iovec(__it,__end, 
               typename std::iterator_traits<Iterator>::iterator_category());
    } 

    // iovec_iterator: iterate over the iovec descriptor of T objects (usually T is char).
    //

    template <typename T>
    struct iovec_iterator : public std::iterator<std::forward_iterator_tag, T>
    {
    protected:
        std::vector<iovec> * m_iovec_p;
        T * m_iterator;
        std::vector<iovec>::size_type m_slot;

    public:
        iovec_iterator()
        : m_iovec_p(0), m_iterator(0), m_slot(0)
        {}
    
        iovec_iterator(std::vector<iovec> &iov)
        : m_iovec_p(&iov), m_iterator(0),  m_slot(0)
        {
            if(iov.empty())
                return;
            for(auto it = iov.begin(), it_e = iov.end(); it != it_e; ++it)
            {
                if(it->iov_len % sizeof(T))
                    throw std::runtime_error("bad iovec_iterator<T>");
            }
            m_iterator = static_cast<T *>(iov[0].iov_base);
        }

        T & operator*()
        {
            assert(m_iterator);
            return *m_iterator;
        }

        T * operator->()
        {
            assert(m_iterator);
            return m_iterator;
        }

        iovec_iterator &
        operator++()
        {
            if (!m_iterator || !m_iovec_p)
                return *this;

            ++m_iterator;
            
            if (reinterpret_cast<char *>(m_iterator) >= 
                    (static_cast<char *>(m_iovec_p->at(m_slot).iov_base) + 
                                         m_iovec_p->at(m_slot).iov_len))
            {
                if(++m_slot >= m_iovec_p->size())
                {
                    m_iovec_p  = 0;
                    m_iterator = 0;
                    m_slot     = 0;
                    return *this;
                }

                m_iterator = static_cast<T *>(m_iovec_p->at(m_slot).iov_base);
            }
            return *this;
        }

        iovec_iterator 
        operator++(int)
        {
            iovec_iterator c(*this);
            this->operator++();
            return c;
        }

        bool operator==(const iovec_iterator &rhs)
        {
            return m_iterator == rhs.m_iterator;
        }    

        bool operator!=(const iovec_iterator &rhs)
        {
            return m_iterator != rhs.m_iterator;
        }
    };

    // return begin/end iterators of a std::vector<iovec>
    //
    
    template <typename T>
    inline iovec_iterator<T>
    get_iovec_iterator(std::vector<iovec> &iov)
    {
        return iovec_iterator<T>(iov);
    }

    template <typename T>
    inline iovec_iterator<T>
    get_iovec_iterator()
    {
        return iovec_iterator<T>();
    }

} // nnamespace more

namespace std 
{
    template <typename CharT, typename Traits>
    inline std::basic_ostream<CharT, Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, const iovec &iov)
    {
        out << '{' << static_cast<void *>(iov.iov_base) << ':' << iov.iov_len << '}';
        return out;
    }
}

#endif /* _MORE_IOVEC_HPP_ */
