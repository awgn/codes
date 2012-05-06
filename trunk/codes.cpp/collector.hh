/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef COLLECTOR_HH
#define COLLECTOR_HH

#include <tr1/memory>    
#include <stdexcept>
#include <map>
#include <set>

namespace more
{
    template <typename T, typename U >
    class collector
    {
    public:
        struct null_deleter
        {
            template <typename Y>
            void operator()(Y *p)  
            {}
        };

        struct basic_deleter
        {
            template <typename Y>
            void operator()(Y *p)  
            {
                m_rmap.erase(p);
                p->~Y();
                operator delete(p);
            }
        };

        explicit collector(const T &i)
        {
            if (m_rmap.find(reinterpret_cast<void *>(this)) != m_rmap.end()) { 
                m_rmap[reinterpret_cast<void *>(this)] = i;
                m_map[i] =  std::tr1::shared_ptr<U>(reinterpret_cast<U *>(this), basic_deleter()); 
            }
            else { 
                m_rmap[reinterpret_cast<void *>(this)] = i;
                m_map[i] =  std::tr1::shared_ptr<U>(reinterpret_cast<U *>(this), null_deleter());
            }
        }

        ~collector()
        {}

        //
        // shared_ptr lacks a release() method to transfer ownership to 
        // a pod pointer when the use_count is equal to 1.
        // 

        static std::set< std::tr1::shared_ptr<U> > *
        garbage()
        {
            static std::set< std::tr1::shared_ptr<U> > * ret = new std::set< std::tr1::shared_ptr<U> >;
            return ret;
        }

        static U *get(const T &key)
        {
            typename std::map<T, std::tr1::shared_ptr<U> >::iterator it = m_map.find(key);
            if ( it == m_map.end())
                throw std::runtime_error("key not found");
            return it->second.get();
        }

        static void *operator new(size_t n)
        {
            void *ret = ::operator new(n);
            m_rmap[ret] = T(); 
            return ret;
        }

        static void operator delete(void *p)
        {
            typename std::map<void *, T>::iterator it = m_rmap.find(p);
            if (it != m_rmap.end()) {
                std::tr1::shared_ptr<U> t;
                t.swap(m_map.find(it->second)->second);
                garbage()->insert(t);
            }
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
            ::operator delete(p);
        }

    private:
        static std::map<T, std::tr1::shared_ptr<U> > m_map;
        static std::map<void *, T> m_rmap;
    };

    template <typename T, typename U>
    std::map<T, std::tr1::shared_ptr<U> > collector<T,U>::m_map __attribute__((init_priority(101)));

    template <typename T, typename U>
    std::map<void *, T> collector<T,U>::m_rmap __attribute((init_priority(101)));

} // namespace more

#endif /* COLLECTOR_HH */
