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

#include <tr1/type_traits>  

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
            unsigned_type m_head;
            unsigned_type m_tail;

            std::vector<value_type> m_storage;

            head_lock_type m_head_lock;
            tail_lock_type m_tail_lock;

            unsigned_type 
            mod_N(unsigned_type n)
            {
                return n & (N-1);
            }
        
        public:
            shared_queue() 
            : m_head(0), m_tail(0), m_storage(N), m_head_lock(), m_tail_lock() 
            {
                static_assert((N & (N-1)) == 0, not_a_power_of_two); 
            }

            ~shared_queue() 
            {}
            
            bool 
            pop_front(T &ret) 
            {
                more::scoped_lock< tail_lock_type > _lock_(m_tail_lock);
                if ( m_tail == m_head )
                   return false;

                ret = m_storage[m_tail];
                m_tail = mod_N(m_tail+1);
                return true;
            }

            bool 
            push_back(const T & elem)  
            {
                more::scoped_lock< head_lock_type > _lock_(m_head_lock);
                unsigned_type next = mod_N(m_head+1);

                if ( next == m_tail )
                   return false;
                
                m_storage[m_head] = elem;
                m_head = next;
                return true;
            }

            void 
            clear() 
            { 
                more::scoped_lock< tail_lock_type > _lock_(m_tail_lock);
                int m_tail = m_head;
            }
            
            bool
            empty() const 
            { 
                return m_head == m_tail; 
            }

            size_type 
            max_size() const 
            { 
                return N; 
            }

            size_type 
            size() const 
            {
                return m_head >= m_tail ? (m_head - m_tail) : (N +m_head - m_tail);
            }
        };

} // namespace more

#endif /* SHARED_QUEUE_HH */

