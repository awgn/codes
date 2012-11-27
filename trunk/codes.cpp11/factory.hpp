/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _MORE_FACTORY_HPP_
#define _MORE_FACTORY_HPP_ 

#include <type_traits>
#include <tuple>
#include <memory>
#include <string>
#include <map>

#include <tuple_ext.hpp>

namespace more { 

    template <typename B>  
    struct factory_base_allocator
    {
        virtual ~factory_base_allocator() 
        {}

        template <typename ...Ts>
        B * alloc(Ts&& ...args)
        {
            std::tuple<typename std::remove_reference<Ts>::type...> tmp(std::forward<Ts>(args)...);
            return xalloc(static_cast<void *>(&tmp));
        }

        virtual B * xalloc(void *) = 0;
    };

    template <typename B, typename E, typename ... Arg>   // B must be a base class of E 
    struct factory_allocator : public factory_base_allocator<B>
    {
        static_assert(std::is_base_of<B,E>::value, "base_of relationship broken");

        template <typename Tuple, int ...S>
        B * talloc(Tuple &t, seq<S...>)
        {
            return new E(std::move(std::get<S>(t))...);
        }

        virtual B * xalloc(void *tmp)
        {
            auto t = static_cast<std::tuple<typename std::remove_reference<Arg>::type...> *>(tmp);
            return talloc(*t, typename gen_forward<sizeof...(Arg)>::type());
        }
    };

    /////////////////////////////////////////////////////////////////////
    // utility: auto-register the allocator of E element (derived from B)
    //                to the F factory

    template <typename ... Tp>
    struct fac_args {};

    template <typename B  /* Base */,  
              typename E  /* element */
              >
    struct factory_register
    {
        static_assert(std::is_base_of<B,E>::value, "base_of relationship broken");

        template <typename F, typename K, typename ...Ti>
        factory_register(F &f, const K &key, fac_args<Ti...> = fac_args<>())
        {
            pack_register(f, key, fac_args<Ti...>());
        }

        template <typename F, typename K, typename ... Ti>
        void pack_register(F &f, const K &key, fac_args<Ti...>)
        {
            f.regist(key, std::unique_ptr<factory_base_allocator<B>>(new typename more::factory_allocator<B, E, Ti...>()));    
        }
    };

    /////////////////////////////////////////
    // factory class: K:key -> B:base_element


    template <typename K, typename B> 
    class factory
    {
    public:

        typedef std::map<K, std::unique_ptr<factory_base_allocator<B>>> map_type;
        typedef typename map_type::const_iterator const_iterator;
        
        factory()  = default;
        ~factory() = default;

        bool
        regist(const K & key, std::unique_ptr<factory_base_allocator<B>> value)
        { 
            return m_map.insert(make_pair(key, std::move(value))).second; 
        }
        
        bool
        unregist(const K &key)
        { 
            return m_map.erase(key) == 1; 
        }

        bool
        is_registered(const K &key) const
        {
            return m_map.count(key) != 0;
        }

        template <typename ...Ti>
        std::unique_ptr<B>
        operator()(const K &key, Ti&& ... arg) const
        {
            auto it = m_map.find(key);
            if (it == m_map.end())
                return std::unique_ptr<B>();

            return std::unique_ptr<B>(it->second->alloc(std::forward<Ti>(arg)...));
        }

        const_iterator
        begin() const
        {
            return m_map.begin();
        }

        const_iterator
        end() const
        {
            return m_map.end();
        }
        
        const_iterator
        cbegin() const
        {
            return m_map.begin();
        }

        const_iterator
        cend() const
        {
            return m_map.end();
        }

    private:
        map_type m_map;
    };

} // namespace more

#endif /* _MORE_FACTORY_HPP_ */
