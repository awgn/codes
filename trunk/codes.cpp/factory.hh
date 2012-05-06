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

#include <static_assert.hh>     // more!

#include <tr1/type_traits>   
#include <tr1/memory>        
#include <string>
#include <map>

namespace more { 

    using std::tr1::shared_ptr;
    
    namespace factory_util {

#ifndef CLASS_REQUIRES_BINARY_CONCEPT
#define CLASS_REQUIRES_BINARY_CONCEPT(t1,t2,C)       \
    template <void (C<t1,t2>::*ptr_value)()> struct class_ ## C {}; \
    typedef class_ ## C< & C<t1,t2>::constraints> C ## _type
#endif

        template <typename B, typename D>
        struct IsBaseOfConcept
        {
            void constraints()
            {  
                static_assert( (std::tr1::is_base_of<B,D>::value) , base_of_concept ); 
            }
        };

        struct null_type
        {};

        template <int n>
        struct int2type {
            enum { value = n };
        };

        template <typename T>
        struct length;

        template < template <typename, typename, typename, typename, typename, typename> class E>
        struct length < E<null_type, null_type, null_type, null_type, null_type, null_type> >
        {
            enum { value = 0 };
        };
        template < template <typename, typename, typename, typename, typename, typename> class E, 
                   typename P0>
        struct length < E<P0, null_type, null_type, null_type, null_type, null_type> >
        {
            enum { value = 1 };
        };
        template < template <typename, typename, typename, typename, typename, typename> class E, 
                   typename P0, typename P1>
        struct length < E<P0, P1, null_type, null_type, null_type, null_type> >
        {
            enum { value = 2 };
        };
        template < template <typename, typename, typename, typename, typename, typename> class E, 
                   typename P0, typename P1, typename P2>
        struct length < E<P0, P1, P2, null_type, null_type, null_type> >
        {
            enum { value = 3 };
        };
        template < template <typename, typename, typename, typename, typename, typename> class E, 
                   typename P0, typename P1, typename P2, typename P3> 
        struct length < E<P0, P1, P2, P3, null_type, null_type> >
        {
            enum { value = 4 };
        };
        template < template <typename, typename, typename, typename, typename, typename> class E, 
                   typename P0, typename P1, typename P2, typename P3, typename P4> 
        struct length < E<P0, P1, P2, P3, P4, null_type> >
        {
            enum { value = 5 };
        };
        template < template <typename, typename, typename, typename, typename, typename> class E, 
                   typename P0, typename P1, typename P2, typename P3, typename P4, typename P5> 
        struct length < E<P0, P1, P2, P3, P4, P5> >
        {
            enum { value = 6 };
        };

    }

    using namespace factory_util;
    template <typename B, typename P0 = factory_util::null_type, 
                          typename P1 = factory_util::null_type, 
                          typename P2 = factory_util::null_type, 
                          typename P3 = factory_util::null_type, 
                          typename P4 = factory_util::null_type >
    struct factory_base_allocator
    {
        virtual ~factory_base_allocator() {}
        virtual B * alloc(P0 p0 = factory_util::null_type(),
                          P1 p1 = factory_util::null_type(), 
                          P2 p2 = factory_util::null_type(),
                          P3 p3 = factory_util::null_type(),
                          P4 p4 = factory_util::null_type()) = 0;
    };

    template <typename B, typename D, typename P0 = factory_util::null_type, 
                                      typename P1 = factory_util::null_type, 
                                      typename P2 = factory_util::null_type, 
                                      typename P3 = factory_util::null_type, 
                                      typename P4 = factory_util::null_type
                                      >   // B is the base class of D 
    struct factory_allocator : public factory_base_allocator<B,P0,P1,P2,P3,P4>
    {
        CLASS_REQUIRES_BINARY_CONCEPT(B,D,IsBaseOfConcept);

        virtual D * alloc(P0 p0, P1 p1,P2 p2,P3 p3,P4 p4)
        {
            return alloc__(factory_util::int2type<factory_util::length<factory_base_allocator<B,P0,P1,P2,P3,P4> >::value >(), p0, p1, p2, p3, p4);
        }

        template <typename T0, typename T1, typename T2, typename T3, typename T4>    
        D * alloc__(factory_util::int2type<1>, T0, T1, T2, T3, T4)
        {   
            return new D;
        }
        template <typename T0, typename T1, typename T2, typename T3, typename T4>    
        D * alloc__(factory_util::int2type<2>, T0 p0, T1, T2, T3, T4)
        {   
            return new D(p0);
        }
        template <typename T0, typename T1, typename T2, typename T3, typename T4>    
        D * alloc__(factory_util::int2type<3>, T0 p0, T1 p1, T2, T3, T4)
        {   
            return new D(p0,p1);
        }
        template <typename T0, typename T1, typename T2, typename T3, typename T4>    
        D * alloc__(factory_util::int2type<4>, T0 p0, T1 p1, T2 p2, T3, T4)
        {   
            return new D(p0,p1,p2);
        }
        template <typename T0, typename T1, typename T2, typename T3, typename T4>    
        D * alloc__(factory_util::int2type<5>, T0 p0, T1 p1, T2 p2, T3 p3, T4)
        {   
            return new D(p0,p1,p2,p3);
        }
        template <typename T0, typename T1, typename T2, typename T3, typename T4>    
        D * alloc__(factory_util::int2type<6>, T0 p0, T1 p1, T2 p2, T3 p3, T4 p4)
        {   
            return new D(p0,p1,p2,p3,p4);
        }
 
    };

    /////////////////////////////////////////////////////////////////////
    // utility: auto-register the allocator of E element (derived from B)
    //                to the F factory

    template <typename B  /* element */,  
              typename E  /* base element */
              >
    struct factory_register
    {
        CLASS_REQUIRES_BINARY_CONCEPT(B,E,IsBaseOfConcept);

        template <typename F /* factory */>
        factory_register(F &f, const std::string &k)
        {
            f.regist(k, new typename more::factory_allocator<B, E>);    
        }        
        template <typename T0, typename F /* factory */>
        factory_register(F &f, const std::string &k, T0)
        {
            f.regist(k, new typename more::factory_allocator<B, E, T0>);    
        }
        template <typename T0, typename T1, typename F /* factory */>
        factory_register(F &f, const std::string &k, T0, T1)
        {
            f.regist(k, new typename more::factory_allocator<B, E, T0, T1>);    
        }
        template <typename T0, typename T1, typename T2, typename F /* factory */>
        factory_register(F &f, const std::string &k, T0, T1, T2)
        {
            f.regist(k, new typename more::factory_allocator<B, E, T0, T1, T2>);    
        }
        template <typename T0, typename T1, typename T2, typename T3, typename F /* factory */>
        factory_register(F &f, const std::string &k, T0, T1 ,T2 ,T3)
        {
            f.regist(k, new typename more::factory_allocator<B, E, T0, T1, T2, T3>);    
        }
        template <typename T0, typename T1, typename T2, typename T3, typename T4, typename F /* factory */>
        factory_register(F &f, const std::string &k, T0, T1, T2, T3, T4)
        {
            f.regist(k, new typename more::factory_allocator<B, E, T0, T1, T2, T3, T4>);    
        }
 
    };

    /////////////////////////////////////////
    // factory class: K:key -> T:base_element

    template <typename K, typename T, typename P0 = factory_util::null_type, 
                                      typename P1 = factory_util::null_type, 
                                      typename P2 = factory_util::null_type, 
                                      typename P3 = factory_util::null_type, 
                                      typename P4 = factory_util::null_type >
    class factory
    {

    public:
        typedef std::map<K, shared_ptr< factory_base_allocator<T,P0,P1,P2,P3,P4> > > factoryMap;

        factory()
        : m_map()
        {}

        ~factory()
        {}

        bool
        regist(const K & key, factory_base_allocator<T,P0,P1,P2,P3,P4> * value)
        { return m_map.insert( make_pair(key, shared_ptr<factory_base_allocator<T,P0,P1,P2,P3,P4> >(value) ) ).second; }
        
        bool
        unregist(const K &key)
        { return m_map.erase(key) == 1; }

        bool
        is_registered(const K &key) const
        {
            typename factoryMap::const_iterator it = m_map.find(key);
            if (it == m_map.end())
                return false;
            return true;
        }

        shared_ptr<T> 
        operator()(const K &key) const
        {
            typename factoryMap::const_iterator it = m_map.find(key);
            if (it == m_map.end())
                return shared_ptr<T>();
            return shared_ptr<T>(it->second->alloc());
        }

        template <typename T0>
        shared_ptr<T> 
        operator()(const K &key, T0 t0) const
        {
            typename factoryMap::const_iterator it = m_map.find(key);
            if (it == m_map.end())
                return shared_ptr<T>();
            return shared_ptr<T>(it->second->alloc(t0));
        }

        template <typename T0, typename T1>
        shared_ptr<T> 
        operator()(const K &key, T0 t0, T1 t1) const
        {
            typename factoryMap::const_iterator it = m_map.find(key);
            if (it == m_map.end())
                return shared_ptr<T>();
            return shared_ptr<T>(it->second->alloc(t0,t1));
        }

        template <typename T0, typename T1, typename T2>
        shared_ptr<T> 
        operator()(const K &key, T0 t0, T1 t1, T2 t2) const
        {
            typename factoryMap::const_iterator it = m_map.find(key);
            if (it == m_map.end())
                return shared_ptr<T>();
            return shared_ptr<T>(it->second->alloc(t0,t1,t2));
        }

        template <typename T0, typename T1, typename T2, typename T3>
        shared_ptr<T> 
        operator()(const K &key, T0 t0, T1 t1, T2 t2, T3 t3) const
        {
            typename factoryMap::const_iterator it = m_map.find(key);
            if (it == m_map.end())
                return shared_ptr<T>();
            return shared_ptr<T>(it->second->alloc(t0,t1,t2,t3));
        }

        template <typename T0, typename T1, typename T2, typename T3, typename T4>
        shared_ptr<T> 
        operator()(const K &key, T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) const
        {
            typename factoryMap::const_iterator it = m_map.find(key);
            if (it == m_map.end())
                return shared_ptr<T>();
            return shared_ptr<T>(it->second->alloc(t0,t1,t2,t3,t4));
        }

    private:
        factoryMap m_map;
    };

} // namespace more

#endif /* _FACTORY_HH_ */
