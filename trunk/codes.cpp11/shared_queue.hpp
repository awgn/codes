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
#include <atomic>

namespace more
{
    namespace details {

        template <typename T>
        struct cacheline
        {
            cacheline()
            : value()
            {}

            T value;

        } __attribute__((aligned(64)));


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
    }

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

            typedef typename details::shared_queue_lock_traits<Producer::producer_value,Lock>::lock_type head_lock_type;
            typedef typename details::shared_queue_lock_traits<Consumer::consumer_value,Lock>::lock_type tail_lock_type;

            typedef unsigned int unsigned_type;

            typedef struct _h { _h() : index(0), lock() {}; std::atomic<unsigned_type> index; head_lock_type lock; } head_cache_type;
            typedef struct _t { _t() : index(0), lock() {}; std::atomic<unsigned_type> index; tail_lock_type lock; } tail_cache_type;

        private:
            std::array<value_type, N> m_storage;

            details::cacheline<head_cache_type> m_head;
            details::cacheline<tail_cache_type> m_tail;

            static unsigned_type
            _S_mod(unsigned_type n)
            {
                return n & (N-1);
            }

        public:
            shared_queue()
            : m_storage(), m_head(), m_tail()
            {
                static_assert((N & (N-1)) == 0, "N not a power of two");
            }

            ~shared_queue()
            {}

            bool
            pop_front(T& ret)
            {
                std::lock_guard<tail_lock_type> _lock_(m_tail.value.lock);
                auto tail = m_tail.value.index.load(std::memory_order_acquire);
                if (tail == m_head.value.index.load(std::memory_order_acquire))
                    return false;

                ret = std::move(m_storage[tail]);
                m_tail.value.index.store(_S_mod(tail+1), std::memory_order_release);
                return true;
            }

            bool
            push_back(const T& elem)
            {
                std::lock_guard<head_lock_type> _lock_(m_head.value.lock);
                auto head = m_head.value.index.load(std::memory_order_acquire);
                auto next = _S_mod(head+1);

                if (next == m_tail.value.index.load(std::memory_order_acquire))
                   return false;

                m_storage[head] = elem;
                m_head.value.index.store(next, std::memory_order_release);
                return true;
            }

            bool
            push_back(T&& elem)
            {
                std::lock_guard<head_lock_type> _lock_(m_head.value.lock);
                auto head = m_head.value.index.load(std::memory_order_acquire);
                auto next = _S_mod(head+1);

                if (next == m_tail.value.index.load(std::memory_order_acquire))
                   return false;

                m_storage[head] = elem;
                m_head.value.index.store(next, std::memory_order_release);
                return true;
            }

            void
            clear()
            {
                std::lock_guard<tail_lock_type> _lock_(m_tail.value.lock);
                for(auto n = m_tail.value.index.load(std::memory_order_acquire);
                      n != m_head.value.index.load(std::memory_order_acquire); ) {
                    m_storage[n] = T();
                    n = _S_mod(n+1);
                }

                m_tail.value.index.store(m_head.value.index.load(std::memory_order_acquire),
                                          std::memory_order_release);
            }

            bool
            empty() const
            {
                return m_head.value.index.load(std::memory_order_acquire) ==
                        m_tail.value.index.load(std::memory_order_acquire);
            }

            size_type
            max_size() const
            {
                return N;
            }

            size_type
            size() const
            {
                int s =  m_head.value.index.load(std::memory_order_acquire) -
                            m_tail.value.index.load(std::memory_order_acquire);
                return s < 0 ? s + N : s;
            }
        };

} // namespace more

#endif /* _SHARED_QUEUE_HPP_ */

