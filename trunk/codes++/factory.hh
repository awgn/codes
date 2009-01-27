/* $Id: factory.hh 6 2009-01-26 10:31:15Z nicola.bonelli $ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _FACTORY_HH_
#define _FACTORY_HH_ 

#include <tr1/memory>
#include <string>
#include <map>

namespace more { 

    using std::tr1::shared_ptr;

    template <typename B>
    struct factory_base_allocator
    {
        virtual ~factory_base_allocator() {}
        virtual B * alloc() = 0;
    };

    template <typename T, typename B>   // B must be the base class of T and T must be default constructible 
    struct factory_alloc : public factory_base_allocator<B>
    {
        virtual T * alloc() {
            return new T;
        }
    };

    template <typename K, typename T>
    class factory
    {

    public:
        typedef std::map<K, shared_ptr<factory_base_allocator<T> > > factoryMap;

        factory()
        : _M_map()
        {}

        ~factory()
        {
            // typename factoryMap::const_iterator it = _M_map.begin();
            // for(;it != _M_map.end(); ++it) 
            //     delete it->second;
        }

        bool
        regist(const K & key, factory_base_allocator<T> * value)
        { return _M_map.insert( make_pair(key, shared_ptr<factory_base_allocator<T> >(value) ) ).second; }

        bool
        unregist(const K &key)
        {
            // typename factoryMap::const_iterator it = _M_map.find(key);
            // if (it == _M_map.end())
            //     return false;
            // delete it->second;

            return _M_map.erase(key) == 1;
        }

        bool
        is_registered(const K &key) const
        {
            typename factoryMap::const_iterator it = _M_map.find(key);
            if (it == _M_map.end())
                return false;
            return true;
        }

        shared_ptr<T> 
        operator()(const K &key) const
        {
            typename factoryMap::const_iterator it = _M_map.find(key);
            if (it == _M_map.end())
                return shared_ptr<T>();
            return shared_ptr<T>(it->second->alloc());
        }

    private:
        factoryMap _M_map;
    };

} // namespace more

#endif /* _FACTORY_HH_ */
