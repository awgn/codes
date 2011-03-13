/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SHARED_QUEUE_HPP_
#define _SHARED_QUEUE_HPP_

#include <type_traits>
#include <thread>
#include <mutex>
#include <array>

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

            bool try_lock()
            {
                return true;
            }
        };

        typedef null_lock lock_type; 
    };

    struct single_producer
    {
        enum { producer_value = 1 };    
    };
    struct multiple_producer
    {
        enum { producer_value = 2 };    
    };
    struct single_consumer
    {
        enum { consumer_value = 1 };    
    };
    struct multiple_consumer
    {
        enum { consumer_value = 2 };    
    };

    template <typename T, unsigned int N = 1024, 
              typename Producer = single_producer, 
              typename Consumer = single_consumer, 
              typename Lock = std::mutex > 
    class shared_queue  
    {
        public:
            typedef typename std::array<T,N>::size_type  size_type;
            typedef typename std::array<T,N>::value_type value_type;
            typedef typename shared_queue_lock_traits<Producer::producer_value,Lock>::lock_type head_lock_type;
            typedef typename shared_queue_lock_traits<Consumer::consumer_value,Lock>::lock_type tail_lock_type;
            
            typedef unsigned int unsigned_type;

        private:
            unsigned_type _M_head;
            unsigned_type _M_tail;

            std::array<value_type, N> _M_storage;

            head_lock_type _M_head_lock;
            tail_lock_type _M_tail_lock;

            static unsigned_type 
            _S_mod(unsigned_type n)
            {
                return n & (N-1);
            }
        
        public:
            shared_queue() 
            : _M_head(0), _M_tail(0), _M_storage(), _M_head_lock(), _M_tail_lock() 
            {
                static_assert((N & (N-1)) == 0, "N is not a power of two"); 
            }

            ~shared_queue() 
            {}


            bool 
            pop_front(T &ret) 
            {
                std::lock_guard<tail_lock_type> _lock_(_M_tail_lock);
                if ( _M_tail == _M_head ) {
                    return false;
                }

                ret = std::move(_M_storage[_M_tail]);
                _M_tail = _S_mod(_M_tail+1);
                return true;
            }

            bool 
            push_back(const T & elem)  
            {
                std::lock_guard< head_lock_type > _lock_(_M_head_lock);
                unsigned_type next = _S_mod(_M_head+1);

                if ( next == _M_tail ) {
                   return false;
                }

                _M_storage[_M_head] = elem;
                _M_head = next;
                return true;
            }

            bool 
            push_back(T && elem)  
            {
                std::lock_guard< head_lock_type > _lock_(_M_head_lock);
                unsigned_type next = _S_mod(_M_head+1);

                if ( next == _M_tail )
                   return false;
                
                _M_storage[_M_head] = std::move(elem);
                _M_head = next;
                return true;
            }

            void 
            clear() 
            { 
                std::lock_guard< tail_lock_type > _lock_(_M_tail_lock);
                while(_M_tail != _M_head) {
                    _M_storage[_M_head] = T();
                    _M_head = _S_mod(_M_head+1);
                }

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

#endif /* _SHARED_QUEUE_HPP_ */

