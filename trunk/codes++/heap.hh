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

    namespace random_access {

        template <typename K, typename V, template <typename Ty, typename Alloc = std::allocator<Ty> > class _Cont>
        class base_heap
        {
        public:
            typedef K               key_type;
            typedef V               mapped_type;
            typedef std::pair<K,V>  value_type;

        private:
            _Cont<value_type>  _M_cont;

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
            : _M_cont()
            {
                // std::make_heap()
            }

            ~base_heap()
            {} 

            void 
            push(const key_type &key, const mapped_type &value)
            {
                _M_cont.push_back(std::make_pair(key,value));
                std::push_heap(_M_cont.begin(), _M_cont.end(), comp());
            }

            value_type
            pop()
            {
                const value_type ret = _M_cont.front(); 
                std::pop_heap(_M_cont.begin(), _M_cont.end(), comp());
                _M_cont.pop_back();   
                return ret; 
            }

            mapped_type 
            pop_value()
            {
                V ret = _M_cont.front().second; 
                std::pop_heap(_M_cont.begin(), _M_cont.end(), comp());
                _M_cont.pop_back();   
                return ret; 
            }

            value_type &
            next() 
            { return _M_cont.front(); }

            const value_type &
            next() const
            { return _M_cont.front(); }

            bool
            empty() const
            { return _M_cont.empty(); } 

        };

        // template alias is not available...
        //

        template <typename K, typename V>
        struct vector_heap : public  base_heap<K, V, std::vector> {};

        template <typename K, typename V>
        struct deque_heap : public base_heap<K, V, std::deque> {};
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
            std::map<key_type, mapped_type, std::less<K> >  _M_cont;

        public:
            heap()
            : _M_cont()
            {}

            ~heap()
            {} 

            void 
            push(const key_type &key, const mapped_type &value)
            {
                _M_cont.insert( std::make_pair(key,value) );
            }

            value_type    
            pop()
            {
                const value_type ret = * _M_cont.begin(); 
                _M_cont.erase(_M_cont.begin());
                return ret; 
            }

            V pop_value()
            {
                V ret = _M_cont.begin()->second; 
                _M_cont.erease(_M_cont.begin());
                return ret; 
            }

            value_type &
            next() 
            { return * _M_cont.begin(); }

            const value_type &
            next() const
            { return * _M_cont.begin(); }

            bool
            empty() const
            { return _M_cont.empty(); } 
 
        };
    }

} // namespace more

#endif /* _HEAP_HH_ */
