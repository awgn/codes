/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _OBSERVER_HH_
#define _OBSERVER_HH_ 

#include <atomicity-policy.hh>

#include <tr1/functional>
#include <tr1/memory>
#include <algorithm>
#include <cassert>

using std::tr1::mem_fn;
using std::tr1::bind;
using namespace std::tr1::placeholders;

namespace more { 

    template <typename T = void>
    class observer
    { 
    public:
        observer()
        {}

        virtual ~observer()
        {}

        void updatex(T x)
        { 
            this->update(x); 
        }

    protected:
        virtual void update(T)=0;
    };

    template <>
    class observer<void>
    { 
    public:
        observer()
        {}

        virtual ~observer()
        {}

        void updatex()
        { 
            this->update(); 
        }

    protected:
        virtual void update()=0;
    };

    template <bool v, typename U, typename V>
    struct select_type
    {
        typedef U type;
    };
    template <typename U, typename V>
    struct select_type<false, U, V>
    {
        typedef V type;
    };

    template <bool v>
    struct bool2type
    { enum { value = v }; };

    template < template <typename Tp, typename Alloc = std::allocator<Tp> > 
               class Cont,                              /* container template */ 
               typename P = void,                       /* update parameter */      
               bool observerOwnership = false           /* onwership policy: false = raw pointers, true = shared_ptr<> */ ,
               typename Atomicity = atomicity::DEFAULT  /* for multithread set to GNU_CXX */ >
    class subject : private atomicity::emptybase_mutex<atomicity::DEFAULT>
    { 
    public:
        typedef typename select_type<observerOwnership, 
                std::tr1::shared_ptr<observer<P> >,
                observer<P> * >::type ptr_type; 

        subject()
        : _M_observers()
        {}

        void notify()
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), mem_fn(&observer<P>::updatex)); 
        }

        template<typename T>
        void notify(T n)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), bind( mem_fn(&observer<P>::updatex), _1, n) ); 
        }

        void attach(ptr_type o)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            if ( std::find( _M_observers.begin(), _M_observers.end(), o ) == _M_observers.end())
                _M_observers.push_back(o);
        }
        void detach(ptr_type o)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            typename Cont<ptr_type>::iterator it = std::find(_M_observers.begin(), _M_observers.end(), o);
            if ( it != _M_observers.end())
                _M_observers.erase(it);
        }

    protected:
        ~subject()
        {}

    private:  
        
        Cont< ptr_type >  _M_observers;
    };

} // namespace more

#endif /* _OBSERVER_HH_ */
