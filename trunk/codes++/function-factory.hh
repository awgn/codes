/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef FUNCTION_FACTORY_HH
#define FUNCTION_FACTORY_HH

#ifdef USE_BOOST
#include <boost/function.hpp>
namespace std { namespace tr1 = ::boost; }
#else
#include <tr1/functional>
#endif

#include <iostream>
#include <stdexcept>
#include <string>
#include <map>

namespace generic 
{
    template <typename Key, typename T>
    class functionFactory
    {
    public:
        typedef std::tr1::function<T> FactoryFunction;
        struct register_factory
        {
            functionFactory<Key,T> & _M_ref;
            FactoryFunction   _M_elem;
            const std::string _M_name;

            template <typename F>
            register_factory(functionFactory &ref, const std::string &name, const F &elem)
            : _M_ref(ref),
              _M_elem(std::tr1::function<T>(elem)),
              _M_name(name)
            {
                _M_ref.regist(_M_name, _M_elem);
            }

            ~register_factory()
            { _M_ref.unregist(_M_name); }

        };

        functionFactory()
        : _M_FactoryMap()
        {}

        ~functionFactory()
        {}

        FactoryFunction 
        operator()(const Key &k) const  
        {
            typename FactoryMap::const_iterator it = _M_FactoryMap.find(k); 
            if ( it == _M_FactoryMap.end() )
                throw std::runtime_error("factory: unknown producer!");    

            return it->second;
        }

        bool 
        has_key(const Key &k) const 
        {
            typename FactoryMap::const_iterator i = _M_FactoryMap.find(k);
            if ( i == _M_FactoryMap.end() )
                return false;
            return true;
        }

        template <typename E>
        bool regist(const Key &k, E &e) 
        {
            return _M_FactoryMap.insert(typename FactoryMap::value_type(k, std::tr1::function<T>(e))).second;
        }

        bool unregist(const Key &k) 
        {
            return _M_FactoryMap.erase(k) == 1;
        }               

    private:
        typedef std::map<Key, FactoryFunction > FactoryMap;
        FactoryMap _M_FactoryMap;

    };

} // namespace generic

#endif /* FUNCTION_FACTORY_HH */
