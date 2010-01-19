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
#include <map>

namespace more { 

    // SGI http://www.sgi.com/tech/stl/make_heap.html: 
    // A heap is a particular way of ordering the elements in a range of Random Access Iterators [f, l)
    // This heap implementation is based on the SGI algorithm make_heap/pop_heap/push_heap.

    struct random_access {

        template <typename K, typename V, template <typename Ty, typename Alloc = std::allocator<Ty> > class __cont = std::vector>
        class heap
        {
        private:
            __cont< std::pair<K,V> >  _M_cont;

            // compare predicate...
            struct comp : std::binary_function<std::pair<K,V>, std::pair<K,V>, bool>
            {
                bool operator()(const std::pair<K,V> &a, const std::pair<K,V> &b) const
                {
                    return a.first > b.first;
                }
            };

        public:
            heap()
            : _M_cont()
            {
                // make_heap()
            }

            ~heap()
            {} 

            void 
            push(const K &key, const V &value)
            {
                _M_cont.push_back(std::make_pair(key,value));
                std::push_heap(_M_cont.begin(), _M_cont.end(), comp());
            }

            std::pair<K,V>    
            pop()
            {
                const std::pair<K,V> ret = _M_cont.front(); 
                std::pop_heap(_M_cont.begin(), _M_cont.end(), comp());
                _M_cont.pop_back();   
                return ret; 
            }

            V pop_value()
            {
                V ret = _M_cont.front().second; 
                std::pop_heap(_M_cont.begin(), _M_cont.end(), comp());
                _M_cont.pop_back();   
                return ret; 
            }

            std::pair<K,V> &
            next() 
            { return _M_cont.front(); }

            const std::pair<K,V> &
            next() const
            { return _M_cont.front(); }
        };

    };

    // SGI: A heap is a particular way of ordering the elements in a range [f, l)
    // this heap implementation is based on std::map<>. Basically it's heap
    // implemented by means of a redblack tree. 

    struct redblack {

        template <typename K, typename V>
        class heap
        {
        private:
            std::map<K,V, std::less<K> >  _M_cont;

        public:
            heap()
            : _M_cont()
            {}

            ~heap()
            {} 

            void 
            push(const K &key, const V &value)
            {
                _M_cont.insert( std::make_pair(key,value) );
            }

            std::pair<K,V>    
            pop()
            {
                const std::pair<K,V> ret = * _M_cont.begin(); 
                _M_cont.erase(_M_cont.begin());
                return ret; 
            }

            V pop_value()
            {
                V ret = _M_cont.begin()->second; 
                _M_cont.erease(_M_cont.begin());
                return ret; 
            }

            std::pair<K,V> &
            next() 
            { return * _M_cont.begin(); }

            const std::pair<K,V> &
            next() const
            { return * _M_cont.begin(); }

        };
    };

} // namespace more

#endif /* _HEAP_HH_ */
