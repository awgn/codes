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
    public:
        friend class enable_entangled_from_this<Tp>;

        entangled_ptr(Tp *p)
        : _M_ptr(p)
        {
            _M_ptr->add_entangled_for(this);
        }

        ~entangled_ptr()
        {   
            if(_M_ptr)
                _M_ptr->remove_entangled_for(this);
        }

        entangled_ptr(entangled_ptr &&rhs)
        : _M_ptr(rhs._M_ptr)
        {
            rhs._M_ptr = 0;
            _M_ptr->remove_entangled_for(&rhs);
            _M_ptr->add_entangled_for(this);
        }

        entangled_ptr(const entangled_ptr &rhs)
        : _M_ptr(rhs._M_ptr)
        {
            _M_ptr->add_entangled_for(this);
        }

        entangled_ptr &
        operator=(const entangled_ptr &rhs)
        {
            _M_ptr->remove_entangled_for(this);
            _M_ptr = rhs._M_ptr;
            _M_ptr->add_entangled_for(this);
        }

        entangled_ptr &
        operator=(entangled_ptr &&rhs)
        {
            _M_ptr->remove_entangled_for(this);
            _M_ptr = rhs._M_ptr;
            rhs._M_ptr = 0;
            _M_ptr->remove_entangled_for(&rhs);
            _M_ptr->add_entangled_for(this);
        }

        Tp&
        operator*() const
        {
            return *_M_ptr;
        }

        Tp*
        operator->() const
        {
            return _M_ptr;
        }

        Tp*
        get() const
        {
            return _M_ptr;
        }

        long use_count() const
        {
            return _M_ptr->use_count();
        }

    private:
        Tp * _M_ptr;
    };


    template <typename Tp>
    class enable_entangled_from_this
    { 
    public:
        typedef enable_entangled_from_this<Tp> super;
        friend class entangled_ptr<Tp>;

        enable_entangled_from_this()
        : _M_ptr()
        {}

        virtual ~enable_entangled_from_this()
        {}

        enable_entangled_from_this(enable_entangled_from_this &&rhs)
        : _M_ptr(std::move(rhs._M_ptr))
        {
            auto it = _M_ptr.begin();
            auto it_e = _M_ptr.end();
            for(; it != it_e; ++it)
            {
                (*it)->_M_ptr = static_cast<Tp *>(this);
            }
        }

        entangled_ptr<Tp>
        entangled_from_this()
        {
            entangled_ptr<Tp> ret(static_cast<Tp *>(this));
            return std::move(ret);
        }

    private:
        // non-copyable idiom
        enable_entangled_from_this(const enable_entangled_from_this &);
        enable_entangled_from_this & operator=(const enable_entangled_from_this &);

        std::vector<entangled_ptr<Tp> *> _M_ptr;

        void add_entangled_for(entangled_ptr<Tp> *p)
        {   
            _M_ptr.push_back(p);
        }

        void remove_entangled_for(entangled_ptr<Tp> *p)
        {
            _M_ptr.erase(std::remove(_M_ptr.begin(), _M_ptr.end(), p), _M_ptr.end());
        }

        long use_count() const
        {
            return _M_ptr.size();
        }
    };

} // namespace more

#endif /* _MAGNETIC_PTR_HPP_ */
