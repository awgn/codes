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

#include <atomicity-policy.hh> // more

#ifndef __GXX_EXPERIMENTAL_CXX0X__
#include <tr1/functional>
#include <tr1/memory>
#include <tr1/type_traits>
namespace std { using namespace std::tr1; }
#else
#include <functional>
#include <memory>
#include <type_traits>

#endif

#include <algorithm>
#include <vector>
#include <cassert>
#include <stdexcept>

using std::mem_fn;
using std::bind;
using namespace std::placeholders;

namespace more { 

    namespace observer_opt {

        template <bool value, typename U, typename V>
        struct select_type
        {
            typedef U type;
        };
        template <typename U, typename V>
        struct select_type<false, U, V>
        {
            typedef V type;
        };

        struct null {};
      
        //////////////////////////////////////////////////////////////////// 
        // enable shared_ptr...

        struct enable_shared_ptr {};  
        
        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5, typename T6>
        struct is_shared_ptr_enabled
        {
            enum { value = std::is_same<T1, enable_shared_ptr>::value ||
                           std::is_same<T2, enable_shared_ptr>::value ||
                           std::is_same<T3, enable_shared_ptr>::value ||
                           std::is_same<T4, enable_shared_ptr>::value ||
                           std::is_same<T5, enable_shared_ptr>::value ||
                           std::is_same<T6, enable_shared_ptr>::value };
        };

        ////////////////////////
        // restore null type...

        template <typename T>
        struct restore_type 
        {
            typedef T value_type;
        };
        template <>
        struct restore_type<enable_shared_ptr>
        {
            typedef null value_type;
        };

    }

    template <typename T1 = observer_opt::null, typename T2 = observer_opt::null, typename T3 = observer_opt::null,
              typename T4 = observer_opt::null, typename T5 = observer_opt::null, typename T6 = observer_opt::null >
    class observer
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

        void updatex(T1 x1)
        { 
            this->update(x1); 
        }
        
        void updatex(T1 x1, T2 x2)
        { 
            this->update(x1,x2); 
        }
        
        void updatex(T1 x1, T2 x2, T3 x3)
        { 
            this->update(x1,x2,x3); 
        }
        
        void updatex(T1 x1, T2 x2, T3 x3, T4 x4)
        { 
            this->update(x1,x2,x3,x4); 
        }
        
        void updatex(T1 x1, T2 x2, T3 x3, T4 x4, T5 x5)
        { 
            this->update(x1,x2,x3,x4,x5); 
        }
        
        void updatex(T1 x1, T2 x2, T3 x3, T4 x4, T5 x5, T6 x6)
        { 
            this->update(x1,x2,x3,x4,x5,x6); 
        }

    protected:
        virtual void update() 
        {
            throw std::runtime_error("subject/observer: slot not implemented");
        }
        virtual void update(T1 x1) 
        {
            throw std::runtime_error("subject/observer: slot not implemented");
        }
        virtual void update(T1 x1, T2 x2) 
        {
            throw std::runtime_error("subject/observer: slot not implemented");
        }
        virtual void update(T1 x1, T2 x2, T3 x3) 
        {
            throw std::runtime_error("subject/observer: slot not implemented");
        }
        virtual void update(T1 x1, T2 x2, T3 x3, T4 x4) 
        {
            throw std::runtime_error("subject/observer: slot not implemented");
        }
        virtual void update(T1 x1, T2 x2, T3 x3, T4 x4, T5 x5) 
        {
            throw std::runtime_error("subject/observer: slot not implemented");
        }
        virtual void update(T1 x1, T2 x2, T3 x3, T4 x4, T5 x5, T6 x6) 
        {
            throw std::runtime_error("subject/observer: slot not implemented");
        }
    };

    template < typename P1 = observer_opt::null, typename P2 = observer_opt::null, /* update parameters */  
               typename P3 = observer_opt::null, typename P4 = observer_opt::null, /* update parameters */
               typename P5 = observer_opt::null, typename P6 = observer_opt::null, /* update parameters */    
               typename Atomicity = atomicity::DEFAULT                             /* for multithread set to GNU_CXX */ >
    class subject : private atomicity::emptybase_mutex<Atomicity>
    { 
    public:

        typedef typename observer_opt::restore_type<P1>::value_type value_type_1;
        typedef typename observer_opt::restore_type<P2>::value_type value_type_2;
        typedef typename observer_opt::restore_type<P3>::value_type value_type_3;
        typedef typename observer_opt::restore_type<P4>::value_type value_type_4;
        typedef typename observer_opt::restore_type<P5>::value_type value_type_5;
        typedef typename observer_opt::restore_type<P6>::value_type value_type_6;

        typedef typename observer_opt::select_type< observer_opt::is_shared_ptr_enabled<P1,P2,P3,P4,P5,P6>::value, 
                std::shared_ptr<observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6> >,
                observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6> * >::type ptr_type; 

        subject()
        : _M_observers()
        {}

        void notify()
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), 
                 mem_fn(static_cast<void(observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>:: *)(void)>
                        (&observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>::updatex))); 
        }
        template<typename T1>
        void notify(T1 n1)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), 
                 bind( mem_fn(
                      static_cast<void(observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>:: *)(T1)>
                      (&observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>::updatex)), _1, n1) ); 
        }
        template<typename T1, typename T2>
        void notify(T1 n1, T2 n2)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), 
                 bind( mem_fn(
                       static_cast<void(observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>:: *)(T1,T2)>
                       (&observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>::updatex)), _1, n1, n2) ); 
        }
        template<typename T1, typename T2, typename T3>
        void notify(T1 n1, T2 n2, T3 n3)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), 
                 bind( mem_fn(
                       static_cast<void(observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>:: *)(T1,T2,T3)>
                       (&observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>::updatex)), _1, n1, n2, n3) ); 
        }
        template<typename T1, typename T2, typename T3,
                 typename T4>
        void notify(T1 n1, T2 n2, T3 n3, T4 n4)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), 
                 bind( mem_fn(
                       static_cast<void(observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>:: *)(T1,T2,T3,T4)>
                       (&observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>::updatex)), _1, n1, n2, n3, n4) ); 
        }
        template<typename T1, typename T2, typename T3,
                 typename T4, typename T5>
        void notify(T1 n1, T2 n2, T3 n3, T4 n4, T5 n5)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), 
                 bind( mem_fn(
                       static_cast<void(observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>:: *)(T1,T2,T3,T4,T5)>
                       (&observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>::updatex)), _1, n1, n2, n3, n4, n5) ); 
        }
        template<typename T1, typename T2, typename T3,
                 typename T4, typename T5, typename T6>
        void notify(T1 n1, T2 n2, T3 n3, T4 n4, T5 n5, T6 n6)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            std::for_each(_M_observers.begin(),  _M_observers.end(), 
                 bind( mem_fn(
                       static_cast<void(observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>:: *)(T1,T2,T3,T4,T5,T6)>
                       (&observer<value_type_1,value_type_2,value_type_3,value_type_4,value_type_5,value_type_6>::updatex)), _1, n1, n2, n3, n4, n5, n6) ); 
        }

        ///////////////////////////////////////////////////////////////////////////////////////

        void attach(ptr_type o)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            if ( std::find( _M_observers.begin(), _M_observers.end(), o ) == _M_observers.end())
                _M_observers.push_back(o);
        }
        void detach(ptr_type o)
        {
            typename Atomicity::scoped_lock lock(this->mutex());
            typename std::vector<ptr_type>::iterator it = std::find(_M_observers.begin(), _M_observers.end(), o);
            if ( it != _M_observers.end())
                _M_observers.erase(it);
        }

    protected:
        ~subject()
        {}

    private:  
        std::vector<ptr_type>  _M_observers;
    };

} // namespace more

#endif /* _OBSERVER_HH_ */
