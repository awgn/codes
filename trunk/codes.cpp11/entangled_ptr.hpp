/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _ENTANGLED_PTR_HPP_
#define _ENTANGLED_PTR_HPP_

#include <stdexcept>
#include <vector>
#include <algorithm>

namespace more {

    template <typename Tp> class enable_entangled_from_this;

    template <typename Tp>
    class entangled_ptr
    {
        entangled_ptr(Tp *p)
        : m_ptr(p)
        {
            m_ptr->m_add_entangled_for(this);
        }

    public:
        friend class enable_entangled_from_this<Tp>;

        entangled_ptr()
        : m_ptr(nullptr)
        {}

        ~entangled_ptr()
        {
            if(m_ptr)
                m_ptr->m_remove_entangled_for(this);
        }

        entangled_ptr(entangled_ptr &&rhs) noexcept
        : m_ptr(rhs.m_ptr)
        {
            rhs.m_ptr = nullptr;
            if(m_ptr)
                m_ptr->m_replace_entangled_for(&rhs,this);
        }

        entangled_ptr(const entangled_ptr &rhs)
        : m_ptr(rhs.m_ptr)
        {
            if(m_ptr)
                m_ptr->m_add_entangled_for(this);
        }

        entangled_ptr &
        operator=(const entangled_ptr &rhs)
        {
            if(m_ptr)
                m_ptr->m_remove_entangled_for(this);
            m_ptr = rhs.m_ptr;
            if(m_ptr)
                m_ptr->m_add_entangled_for(this);
            return *this;
        }

        entangled_ptr &
        operator=(entangled_ptr &&rhs) noexcept
        {
            if(m_ptr)
                m_ptr->m_remove_entangled_for(this);
            m_ptr = rhs.m_ptr;
            rhs.m_ptr = nullptr;
            if(m_ptr)
                m_ptr->m_replace_entangled_for(&rhs,this);
            return *this;
        }

        Tp&
        operator*() const
        {
            return *m_check_ptr(m_ptr);
        }

        Tp*
        operator->() const
        {
            return m_check_ptr(m_ptr);
        }

        Tp*
        get() const
        {
            return m_ptr;
        }

        size_t
        use_count() const
        {
            return m_check_ptr(m_ptr)->m_use_count();
        }

    private:
        static Tp * m_check_ptr(Tp *ptr)
        {
            if(!ptr)
                throw std::runtime_error("entangled_ptr::m_check_ptr");
            return ptr;
        }

        Tp * m_ptr;
    };


    template <typename Tp>
    class enable_entangled_from_this
    {
    public:
        typedef enable_entangled_from_this<Tp> super;
        friend class entangled_ptr<Tp>;

        enable_entangled_from_this()
        : m_ref()
        {}

        ~enable_entangled_from_this()
        {
            m_set_entangled_to(nullptr);
        }

        enable_entangled_from_this(enable_entangled_from_this &&rhs)
        : m_ref(std::move(rhs.m_ref))
        {
            m_set_entangled_to(static_cast<Tp *>(this));
        }

        enable_entangled_from_this&
        operator=(enable_entangled_from_this &&rhs)
        {
            m_set_entangled_to(nullptr);
            m_ref = std::move(rhs.m_ref);
            m_set_entangled_to(static_cast<Tp *>(this));
            return *this;
        }

        entangled_ptr<Tp>
        entangled_from_this()
        {
            return entangled_ptr<Tp>(static_cast<Tp *>(this));
        }

    private:
        std::vector<entangled_ptr<Tp> *> m_ref;

        enable_entangled_from_this(const enable_entangled_from_this &) = delete;
        enable_entangled_from_this & operator=(const enable_entangled_from_this &) = delete;

        void m_set_entangled_to(Tp *ptr)
        {
            auto it = m_ref.begin(),
                 it_e = m_ref.end();
            for(; it != it_e; ++it)
            {
                (*it)->m_ptr = ptr;
            }
        }

        void m_add_entangled_for(entangled_ptr<Tp> *ptr)
        {
            m_ref.push_back(ptr);
        }

        void m_remove_entangled_for(entangled_ptr<Tp> *ptr)
        {
            m_ref.erase(std::remove(m_ref.begin(), m_ref.end(), ptr), m_ref.end());
        }

        void m_replace_entangled_for(entangled_ptr<Tp> *ptr_old, entangled_ptr<Tp> *ptr_new) noexcept
        {
            std::replace(m_ref.begin(), m_ref.end(), ptr_old, ptr_new);
        }

        size_t
        m_use_count() const
        {
            return m_ref.size();
        }
    };

    // template <typename CharT, typename Traits, typename Tp>
    // typename std::basic_ostream<CharT, Traits> &
    // operator<<(std::basic_ostream<CharT,Traits> &out, const enable_entangled_from_this<Tp> &rhs)
    // {
    //     out << "[";
    //     std::copy(rhs.m_ref.begin(), rhs.m_ref.end(), std::ostream_iterator<void *>(std::cout, " - "));
    //     return out << "]";
    // }

} // namespace more

#endif /* _ENTANGLED_PTR_HPP_ */
