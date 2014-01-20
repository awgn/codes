/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _VALUE_PTR_HPP_
#define _VALUE_PTR_HPP_

// this is value_ptr implementation
// inspired to that of Sutter (More Exceptional C++)

#include <algorithm>

namespace more {

    /////////////////// traits //////////////////////////

    struct noncopyable_ptr  {};
    struct deepcopyable_ptr {};
    struct cloneable_ptr    {};    // polymorphic constructor (virtual constructor idiom)
    struct transfer_ptr     {};    // ala std::auto_ptr

    template <typename Tp, typename S>
    struct value_ptr_traits;

    template <typename Tp>
    struct value_ptr_traits<Tp, noncopyable_ptr>
    {
    };
    template <typename Tp>
    struct value_ptr_traits<Tp, deepcopyable_ptr>
    {
        static Tp * copy(const Tp *p)
        {
            return p ? new Tp(*p) : 0;
        }
    };
    template <typename Tp>
    struct value_ptr_traits<Tp, cloneable_ptr>
    {
        static Tp * copy(const Tp *p)
        {
            return p ? p->clone() : 0;
        }
    };
    template <typename Tp>
    struct value_ptr_traits<Tp, transfer_ptr>
    {
        static Tp * copy(Tp * &p)
        {
            Tp * ret = p; p = 0;
            return ret;
        }
    };

    /////////////////////////////////////////////////////

    template <typename Tp, typename S = noncopyable_ptr >
    class value_ptr
    {
    private:
        Tp * m_ptr;

    public:
        value_ptr(Tp *ptr = 0)
        : m_ptr(ptr)
        {}

        ~value_ptr()
        {}

        value_ptr(const value_ptr &other)
        : m_ptr( value_ptr_traits<Tp,S>::copy(other.m_ptr) )
        {}
        value_ptr(value_ptr &other)
        : m_ptr( value_ptr_traits<Tp,S>::copy(other.m_ptr) )
        {}

        value_ptr& operator=(const value_ptr &other)
        {
            value_ptr tmp(other);
            tmp.swap(*this);
            return *this;
        }
        value_ptr& operator=(value_ptr &other)
        {
            value_ptr tmp(other);
            tmp.swap(*this);
            return *this;
        }

        //// template version for convertible pointers

        template <typename U>
        value_ptr(const value_ptr<U> &other)
        : m_ptr( value_ptr_traits<U,S>::copy(other.m_ptr) )
        {}
        template <typename U>
        value_ptr(value_ptr<U> &other)
        : m_ptr( value_ptr_traits<U,S>::copy(other.m_ptr) )
        {}

        template <typename U>
        value_ptr& operator=(const value_ptr<U> &other)
        {
            value_ptr tmp(other);
            tmp.swap(*this);
            return *this;
        }
        template <typename U>
        value_ptr& operator=(value_ptr<U> &other)
        {
            value_ptr tmp(other);
            tmp.swap(*this);
            return *this;
        }

        Tp&
        operator*() const
        {
            return *m_ptr;
        }

        Tp*
        operator->() const
        {
            return m_ptr;
        }

        Tp *
        get() const
        {
            return m_ptr;
        }

        void
        swap(value_ptr &other)
        {
            std::swap(m_ptr,other.m_ptr);
        }
    };

} // namespace more

#endif /* _VALUE_PTR_HPP_ */
