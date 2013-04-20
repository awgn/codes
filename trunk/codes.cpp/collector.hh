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
            void operator()(Y *)  
            {}
        };

        struct basic_deleter
        {
            template <typename Y>
            void operator()(Y *p)  
            {
                s_rmap().erase(p);
                p->~Y();
                operator delete(p);
            }
        };

        explicit collector(const T &i)
        {
            if (s_rmap().find(reinterpret_cast<void *>(this)) != s_rmap().end()) { 
                s_rmap()[reinterpret_cast<void *>(this)] = i;
                s_map()[i] =  std::tr1::shared_ptr<U>(reinterpret_cast<U *>(this), basic_deleter()); 
            }
            else { 
                s_rmap()[reinterpret_cast<void *>(this)] = i;
                s_map()[i] =  std::tr1::shared_ptr<U>(reinterpret_cast<U *>(this), null_deleter());
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
            typename std::map<T, std::tr1::shared_ptr<U> >::iterator it = s_map().find(key);
            if ( it == s_map().end())
                throw std::runtime_error("key not found");
            return it->second.get();
        }

        static void *operator new(size_t n)
        {
            void *ret = ::operator new(n);
            s_rmap()[ret] = T(); 
            return ret;
        }

        static void operator delete(void *p)
        {
            typename std::map<void *, T>::iterator it = s_rmap().find(p);
            if (it != s_rmap().end()) {
                std::tr1::shared_ptr<U> t;
                t.swap(s_map().find(it->second)->second);
                garbage()->insert(t);
            }
            // std::cout << __PRETTY_FUNCTION__ << std::endl;
            ::operator delete(p);
        }

    private:

        static std::map<T, std::tr1::shared_ptr<U> > &
        s_map()
        {
            static std::map<T, std::tr1::shared_ptr<U> > ret;
            return ret;
        }

        static std::map<void *, T> &
        s_rmap()
        {
            static std::map<void *, T> ret;
            return ret;
        }
    };

} // namespace more

#endif /* COLLECTOR_HH */
