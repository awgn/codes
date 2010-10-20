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
        : _M_ptr(p)
        {
            _M_ptr->_M_add_entangled_for(this);
        }

    public:
        friend class enable_entangled_from_this<Tp>;

        ~entangled_ptr()
        {   
            if(_M_ptr)
                _M_ptr->_M_remove_entangled_for(this);
        }

        entangled_ptr(entangled_ptr &&rhs)
        : _M_ptr(rhs._M_ptr)
        {
            rhs._M_ptr = 0;
            _M_ptr->_M_replace_entangled_for(&rhs,this);
        }

        entangled_ptr(const entangled_ptr &rhs)
        : _M_ptr(rhs._M_ptr)
        {
            _M_ptr->_M_add_entangled_for(this);
        }

        entangled_ptr &
        operator=(const entangled_ptr &rhs)
        {
            _M_ptr->_M_remove_entangled_for(this);
            _M_ptr = rhs._M_ptr;
            _M_ptr->_M_add_entangled_for(this);
            return *this;
        }

        entangled_ptr &
        operator=(entangled_ptr &&rhs)
        {
            _M_ptr->_M_remove_entangled_for(this);
            _M_ptr = rhs._M_ptr;
            rhs._M_ptr = 0;
            _M_ptr->_M_replace_entangled_for(&rhs,this);
            return *this;
        }

        Tp&
        operator*() const
        {
            return *_M_check_ptr(_M_ptr);
        }

        Tp*
        operator->() const
        {
            return _M_check_ptr(_M_ptr);
        }

        Tp*
        get() const
        {
            return _M_ptr;
        }

        long use_count() const
        {
            return _M_check_ptr(_M_ptr)->_M_use_count();
        }

    private:
        static Tp * _M_check_ptr(Tp *ptr)
        {
            if(!ptr)
                throw std::runtime_error("entangled_ptr::_M_check_ptr");
            return ptr;
        }

        Tp * _M_ptr;
    };


    template <typename Tp>
    class enable_entangled_from_this
    { 
    public:
        typedef enable_entangled_from_this<Tp> super;
        friend class entangled_ptr<Tp>;

        enable_entangled_from_this()
        : _M_ref()
        {}

        ~enable_entangled_from_this()
        {
            _M_set_entangled_to(0);
        }

        enable_entangled_from_this(enable_entangled_from_this &&rhs)
        : _M_ref(std::move(rhs._M_ref))
        {
            _M_set_entangled_to(static_cast<Tp *>(this));
        }

        enable_entangled_from_this&
        operator=(enable_entangled_from_this &&rhs)
        {
            _M_set_entangled_to(0);
            _M_ref = std::move(rhs._M_ref);
            _M_set_entangled_to(static_cast<Tp *>(this));
            return *this;
        }

        entangled_ptr<Tp>
        entangled_from_this()
        {
            entangled_ptr<Tp> ret(static_cast<Tp *>(this));
            return std::move(ret);
        }

    private:
        std::vector<entangled_ptr<Tp> *> _M_ref;
        
        enable_entangled_from_this(const enable_entangled_from_this &) = delete;
        enable_entangled_from_this & operator=(const enable_entangled_from_this &) = delete;

        void _M_set_entangled_to(Tp *ptr)
        {
            auto it = _M_ref.begin();
            auto it_e = _M_ref.end();
            for(; it != it_e; ++it)
            {
                (*it)->_M_ptr = ptr;
            }
        }

        void _M_add_entangled_for(entangled_ptr<Tp> *ptr)
        {                     
            _M_ref.push_back(ptr);
        }

        void _M_remove_entangled_for(entangled_ptr<Tp> *ptr)
        {
            _M_ref.erase(std::remove(_M_ref.begin(), _M_ref.end(), ptr), _M_ref.end());
        }

        void _M_replace_entangled_for(entangled_ptr<Tp> *ptr_old, entangled_ptr<Tp> *ptr_new)
        {                                                    
            std::replace(_M_ref.begin(), _M_ref.end(), ptr_old, ptr_new);
        }

        long _M_use_count() const
        {
            return _M_ref.size();
        }
    };

    // template <typename CharT, typename Traits, typename Tp>
    // typename std::basic_ostream<CharT, Traits> &
    // operator<<(std::basic_ostream<CharT,Traits> &out, const enable_entangled_from_this<Tp> &rhs)
    // {
    //     out << "[";
    //     std::copy(rhs._M_ref.begin(), rhs._M_ref.end(), std::ostream_iterator<void *>(std::cout, " - "));
    //     return out << "]";
    // }

} // namespace more

#endif /* _ENTANGLED_PTR_HPP_ */
