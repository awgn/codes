/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _VARIANT_MUTEX_HPP_
#define _VARIANT_MUTEX_HPP_ 

#include <cassert>
#include <stdexcept>
#include <functional>

namespace more { 

    namespace detail 
    {
        // template metafunction that returns the max sizeof() of a list of types.
        //

        template <typename ...Ts> struct max_sizeof;
        template <typename T, typename ...Ti>
        struct max_sizeof<T, Ti...>
        {
            enum { tail  = max_sizeof<Ti...>::value };
            enum { value = sizeof(T) > static_cast<int>(tail) ?  sizeof(T) : static_cast<int>(tail) };
        };

        template <typename T>
        struct max_sizeof<T>
        {
            enum { value = sizeof(T) };
        };
        
        template <typename Fun, typename ...Tp> struct apply;
        template <typename Fun, typename T, typename ...Tp>
        struct apply<Fun, T, Tp...> 
        {
            static 
            typename std::result_of<Fun(T*)>::type 
            on(char *s, unsigned int type, unsigned int t)
            {
                if (t == type) 
                { 
                    return Fun()(reinterpret_cast<T *>(s)); 
                }
                else
                    return apply<Fun, Tp...>::on(s,type,t+1);
            }
        };
        template <typename Fun, typename T>
        struct apply<Fun, T>
        {
            static 
            typename std::result_of<Fun(T*)>::type 
            on(char *s, unsigned int, unsigned int)
            {
                return Fun()(reinterpret_cast<T *>(s)); 
            }
        };
    }
     
    // Variant mutex for Lockable object. 
    // Requirement 30.2.5.3 Draft, Standard for C++ Programming Language

    template <typename ...Ts>
    class variant_mutex
    {
        struct ctor_action
        {
            typedef void result_type;
            template <typename Tx>
            void operator()(Tx *that)
            {
                new (that)Tx;
            }
        };

        struct dtor_action
        {
            typedef void result_type;
            template <typename Tx>
            void operator()(Tx *that)
            {                     
                that->~Tx();
            }
        };

        struct lock_action
        {
            typedef void result_type;
            template <typename Tx>
            void operator()(Tx *that)
            {
                that->lock();
            }
        };

        struct unlock_action
        {
            typedef void result_type;
            template <typename Tx>
            void operator()(Tx *that)
            {
                that->unlock();
            }
        };

        struct try_lock_action
        {
            typedef bool result_type;
            template <typename Tx>
            bool operator()(Tx *that)
            {
                return that->try_lock();
            }
        };

    public:
        variant_mutex(unsigned int n)
        : m_type(n)
        {
            if (n >= sizeof...(Ts))
                throw std::runtime_error("variant_mutex: bad index");                

            detail::apply<ctor_action, Ts...>::on(m_storage, m_type, 0);
        }

        ~variant_mutex()
        {
            detail::apply<dtor_action, Ts...>::on(m_storage, m_type, 0);
        }

        variant_mutex(const variant_mutex &) = delete;
        variant_mutex& operator=(const variant_mutex &) = delete;

        void lock()
        {
            detail::apply<lock_action, Ts...>::on(m_storage, m_type, 0);
        }

        void unlock()
        {
            detail::apply<unlock_action, Ts...>::on(m_storage, m_type, 0);
        }

        bool try_lock()
        {
            return detail::apply<try_lock_action, Ts...>::on(m_storage, m_type, 0);
        }

    private:
        const unsigned int m_type;
        char m_storage[ detail::max_sizeof<Ts...>::value ];
    };

} // namespace more

#endif /* _VARIANT_MUTEX_HPP_ */
