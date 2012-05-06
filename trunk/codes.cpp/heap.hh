/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _HEAP_HH_
#define _HEAP_HH_ 

#include <functional>
#include <algorithm>
#include <vector>
#include <deque>
#include <queue>
#include <memory>
#include <map>

namespace more { 

    // SGI http://www.sgi.com/tech/stl/make_heap.html: 
    // A heap is a particular way of ordering the elements in a range of Random Access Iterators [f, l)
    // This heap implementation is based on the SGI algorithm make_heap/pop_heap/push_heap.

    namespace random_access {

        template <typename K, typename V, template <typename Ty, typename Alloc = std::allocator<Ty> > class _Cont>
        class base_heap
        {
        public:
            typedef K               key_type;
            typedef V               mapped_type;
            typedef std::pair<K,V>  value_type;

        private:
            _Cont<value_type>  m_cont;

            // compare predicate...
            //
            struct comp : std::binary_function<value_type,value_type,bool>
            {
                bool operator()(const value_type &a, const value_type &b) const
                {
                    return a.first > b.first;
                }
            };

        public:
            base_heap()
            : m_cont()
            {
                // std::make_heap()
            }

            ~base_heap()
            {} 

            void 
            push(const key_type &key, const mapped_type &value)
            {
                m_cont.push_back(std::make_pair(key,value));
                std::push_heap(m_cont.begin(), m_cont.end(), comp());
            }

            value_type
            pop()
            {
                const value_type ret = m_cont.front(); 
                std::pop_heap(m_cont.begin(), m_cont.end(), comp());
                m_cont.pop_back();   
                return ret; 
            }

            mapped_type 
            pop_value()
            {
                V ret = m_cont.front().second; 
                std::pop_heap(m_cont.begin(), m_cont.end(), comp());
                m_cont.pop_back();   
                return ret; 
            }

            value_type &
            top() 
            { return m_cont.front(); }

            const value_type &
            top() const
            { return m_cont.front(); }

            bool
            empty() const
            { return m_cont.empty(); } 

        };

        // template alias is not available...
        //

        template <typename K, typename V>
        struct vector_heap : public  base_heap<K, V, std::vector> {};

        template <typename K, typename V>
        struct deque_heap : public base_heap<K, V, std::deque> {};


        // std::priority_queue adapter...
        //

        template <typename K, typename V>
        struct priority_queue_heap 
        {        
        public:
            typedef K               key_type;
            typedef V               mapped_type;
            typedef std::pair<K,V>  value_type;

        private:
            // compare predicate...
            //
            struct comp : std::binary_function<value_type,value_type,bool>
            {
                bool operator()(const value_type &a, const value_type &b) const
                {
                    return a.first > b.first;
                }
            };

            std::priority_queue<value_type, std::vector<value_type>, comp>  m_pq;

        public:
            priority_queue_heap()
            : m_pq()
            {
                // std::make_heap()
            }

            ~priority_queue_heap()
            {} 

            void 
            push(const key_type &key, const mapped_type &value)
            {
                m_pq.push(std::make_pair(key,value));
            }

            value_type
            pop()
            {
                const value_type ret = m_pq.top(); 
                m_pq.pop();   
                return ret; 
            }

            mapped_type 
            pop_value()
            {
                V ret = m_pq.top().second; 
                m_pq.pop();   
                return ret; 
            }

            value_type &
            top() 
            { return m_pq.top(); }

            const value_type &
            top() const
            { return m_pq.top(); }

            bool
            empty() const
            { return m_pq.empty(); } 

        };
    }

    // SGI: A heap is a particular way of ordering the elements in a range [f, l)
    // this heap implementation is based on std::map<>. Basically it's heap
    // implemented by means of a redblack tree. 

    namespace redblack {

        template <typename K, typename V>
        class heap
        {        
        public:
            typedef K               key_type;
            typedef V               mapped_type;
            typedef std::pair<K,V>  value_type;

        private:
            std::map<key_type, mapped_type, std::less<K> >  m_cont;

        public:
            heap()
            : m_cont()
            {}

            ~heap()
            {} 

            void 
            push(const key_type &key, const mapped_type &value)
            {
                m_cont.insert( std::make_pair(key,value) );
            }

            value_type    
            pop()
            {
                const value_type ret = * m_cont.begin(); 
                m_cont.erase(m_cont.begin());
                return ret; 
            }

            V pop_value()
            {
                V ret = m_cont.begin()->second; 
                m_cont.erease(m_cont.begin());
                return ret; 
            }

            value_type &
            top() 
            { return * m_cont.begin(); }

            const value_type &
            top() const
            { return * m_cont.begin(); }

            bool
            empty() const
            { return m_cont.empty(); } 
 
        };
    }

} // namespace more

#endif /* _HEAP_HH_ */
