/* ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ITERATOR_HPP_
#define _ITERATOR_HPP_

#include <iterator>
#include <type_traits>
#include <tuple>

namespace more {

    template <typename Iter, size_t N>
    struct tuple_adapter_iterator
    {
        typedef typename Iter::iterator_category                        iterator_category;
        typedef typename Iter::difference_type                          difference_type;
        typedef typename std::tuple_element<N, typename Iter::value_type>::type  value_type;
        typedef typename std::add_pointer<value_type>::type              pointer;
        typedef typename std::add_lvalue_reference<value_type>::type    reference;

        tuple_adapter_iterator(Iter it)
        : it_(it)
        {}

        ~tuple_adapter_iterator()
        {}

        reference
        operator *() const
        {
            return std::get<N>(*it_);
        }

        pointer
        operator->() const
        {
            return &std::get<N>(*it_);
        }

        tuple_adapter_iterator&
        operator++()
        {
            ++it_;
            return *this;
        }

        tuple_adapter_iterator
        operator++(int)
        {
            tuple_adapter_iterator ret(*this);
            ++(*this);
            return ret;
        }

        tuple_adapter_iterator&
        operator--()
        {
            --it_;
            return *this;
        }

        tuple_adapter_iterator
        operator--(int)
        {
            tuple_adapter_iterator ret(*this);
            --(*this);
            return ret;
        }

        friend bool operator==(const tuple_adapter_iterator &lhs, const tuple_adapter_iterator &rhs)
        {
            return lhs.it_ == rhs.it_;
        }

        friend bool operator!=(const tuple_adapter_iterator &lhs, const tuple_adapter_iterator &rhs)
        {
            return lhs.it_ != rhs.it_;
        }

        // random access iterator requirements
        //

        reference
        operator[](const difference_type &n) const
        {
            return std::get<N>(it_[n]);
        }

        tuple_adapter_iterator &
        operator+=(int n)
        {
            it_ += n;
            return *this;
        }

        friend const tuple_adapter_iterator operator+(tuple_adapter_iterator lhs, int n)
        { return lhs+=n; }

        tuple_adapter_iterator &
        operator-=(int n)
        {
            it_ -= n;
            return *this;
        }

        friend const tuple_adapter_iterator operator-(tuple_adapter_iterator lhs, int n)
        { return lhs-=n; }

        friend difference_type operator-(const tuple_adapter_iterator& lhs,
                                         const tuple_adapter_iterator& rhs)
        {
            return lhs.it_-rhs.it_;
        }

        friend bool operator<(const tuple_adapter_iterator& lhs,
                              const tuple_adapter_iterator& rhs)
        {
            return lhs.it_ < rhs.it_;
        }
        friend bool operator> (const tuple_adapter_iterator& lhs,
                               const tuple_adapter_iterator& rhs)
        {
            return rhs < lhs;
        }
        friend bool operator>=(const tuple_adapter_iterator& lhs,
                               const tuple_adapter_iterator& rhs)
        {
            return !(lhs<rhs);
        }
        friend bool operator<=(const tuple_adapter_iterator& lhs,
                               const tuple_adapter_iterator& rhs)
        {
            return !(lhs>rhs);
        }
    private:

        Iter it_;
    };


    template <typename Iter>
    auto first_iterator(Iter it)
    -> tuple_adapter_iterator<Iter,0>
    {
        return tuple_adapter_iterator<Iter,0>(it);
    }

    template <typename Iter>
    auto second_iterator(Iter it)
    -> tuple_adapter_iterator<Iter,1>
    {
        return tuple_adapter_iterator<Iter,1>(it);
    }

    template <size_t N, typename Iter>
    auto tuple_element_iterator(Iter it)
    -> tuple_adapter_iterator<Iter, N>
    {
        return tuple_adapter_iterator<Iter,N>(it);
    }


    template <typename Iter>
    struct address_adapter_iterator
    {
        typedef typename Iter::iterator_category                        iterator_category;
        typedef typename Iter::difference_type                          difference_type;
        typedef typename std::add_pointer<typename Iter::value_type>::type    value_type;
        typedef typename std::add_pointer<value_type>::type             pointer;
        typedef typename std::add_const<value_type>::type               reference;

        address_adapter_iterator(Iter it)
        : it_(it)
        {}

        ~address_adapter_iterator()
        {}

        reference
        operator *() const
        {
            return &(*it_);
        }

        pointer
        operator->() const
        {
            return &(&(*it_));
        }

        address_adapter_iterator&
        operator++()
        {
            ++it_;
            return *this;
        }

        address_adapter_iterator
        operator++(int)
        {
            address_adapter_iterator ret(*this);
            ++(*this);
            return ret;
        }

        address_adapter_iterator&
        operator--()
        {
            --it_;
            return *this;
        }

        address_adapter_iterator
        operator--(int)
        {
            address_adapter_iterator ret(*this);
            --(*this);
            return ret;
        }

        friend bool operator==(const address_adapter_iterator &lhs, const address_adapter_iterator &rhs)
        {
            return lhs.it_ == rhs.it_;
        }

        friend bool operator!=(const address_adapter_iterator &lhs, const address_adapter_iterator &rhs)
        {
            return lhs.it_ != rhs.it_;
        }

        // random access iterator requirements
        //

        reference
        operator[](const difference_type &n) const
        {
            return &(*it_[n]);
        }

        address_adapter_iterator &
        operator+=(int n)
        {
            it_ += n;
            return *this;
        }

        friend const address_adapter_iterator operator+(address_adapter_iterator lhs, int n)
        { return lhs+=n; }

        address_adapter_iterator &
        operator-=(int n)
        {
            it_ -= n;
            return *this;
        }

        friend const address_adapter_iterator operator-(address_adapter_iterator lhs, int n)
        { return lhs-=n; }

        friend difference_type operator-(const address_adapter_iterator& lhs,
                                         const address_adapter_iterator& rhs)
        {
            return lhs.it_ - rhs.it_;
        }

        friend bool operator<(const address_adapter_iterator& lhs,
                              const address_adapter_iterator& rhs)
        {
            return lhs.it_ < rhs.it_;
        }
        friend bool operator> (const address_adapter_iterator& lhs,
                               const address_adapter_iterator& rhs)
        {
            return rhs < lhs;
        }
        friend bool operator>=(const address_adapter_iterator& lhs,
                               const address_adapter_iterator& rhs)
        {
            return !(lhs<rhs);
        }
        friend bool operator<=(const address_adapter_iterator& lhs,
                               const address_adapter_iterator& rhs)
        {
            return !(lhs>rhs);
        }
    private:

        Iter it_;
    };

    template <typename Iter>
    auto address_iterator(Iter it)
    -> address_adapter_iterator<Iter>
    {
        return address_adapter_iterator<Iter>(it);
    }
} // namespace more


#endif /* _ITERATOR_HPP_ */
