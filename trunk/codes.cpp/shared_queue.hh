/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef SHARED_QUEUE_HH
#define SHARED_QUEUE_HH

#include <noncopyable.hh>       // more!
#include <static_assert.hh>     // more!
#include <spinlock.hh>          // more!
#include <tr1_type_traits.hh>   // more!

#include <vector>

namespace more 
{
    template <int N, typename Lock>    
    struct shared_queue_lock_traits
    {
        typedef Lock lock_type;
    };

    template <typename Lock> 
    struct shared_queue_lock_traits<0,Lock>;
    template <typename Lock>
    struct shared_queue_lock_traits<1, Lock>
    {
        struct null_lock
        {
            void lock()
            {}

            void unlock()
            {}
        };

        typedef null_lock lock_type; 
    };

    template <bool N> struct shared_queue_element;
    template <>
    struct shared_queue_element<true>
    {};

    template <typename T, unsigned int N = 1024, 
              unsigned int Producer = 1, unsigned int Consumer = 1, 
              typename Lock = more::spinlock<more::lock_backoff<> > > 
    class shared_queue : more::noncopyable, 
                         shared_queue_element< std::tr1::has_nothrow_assign<T>::value && std::tr1::has_nothrow_copy<T>::value  >
    {
        public:
            typedef typename std::vector<T>::size_type  size_type;
            typedef typename std::vector<T>::value_type value_type;
            typedef typename shared_queue_lock_traits<Producer,Lock>::lock_type head_lock_type;
            typedef typename shared_queue_lock_traits<Consumer,Lock>::lock_type tail_lock_type;
            typedef unsigned int unsigned_type;

        private:
            unsigned_type _M_head;
            unsigned_type _M_tail;

            std::vector<value_type> _M_storage;

            head_lock_type _M_head_lock;
            tail_lock_type _M_tail_lock;

            unsigned_type 
            mod_N(unsigned_type n)
            {
                return n & (N-1);
            }
        
        public:
            shared_queue() 
            : _M_head(0), _M_tail(0), _M_storage(N), _M_head_lock(), _M_tail_lock() 
            {
                static_assert((N & (N-1)) == 0, not_a_power_of_two); 
            }

            ~shared_queue() 
            {}
            
            bool 
            pop_front(T &ret) 
            {
                more::scoped_lock< tail_lock_type > _lock_(_M_tail_lock);
                if ( _M_tail == _M_head )
                   return false;

                ret = _M_storage[_M_tail];
                _M_tail = mod_N(_M_tail+1);
                return true;
            }

            bool 
            push_back(const T & elem)  
            {
                more::scoped_lock< head_lock_type > _lock_(_M_head_lock);
                unsigned_type next = mod_N(_M_head+1);

                if ( next == _M_tail )
                   return false;
                
                _M_storage[_M_head] = elem;
                _M_head = next;
                return true;
            }

            void 
            clear() 
            { 
                more::scoped_lock< tail_lock_type > _lock_(_M_tail_lock);
                int _M_tail = _M_head;
            }
            
            bool
            empty() const 
            { 
                return _M_head == _M_tail; 
            }

            size_type 
            max_size() const 
            { 
                return N; 
            }

            size_type 
            size() const 
            {
                return _M_head >= _M_tail ? (_M_head - _M_tail) : (N +_M_head - _M_tail);
            }
        };

} // namespace more

#endif /* SHARED_QUEUE_HH */

