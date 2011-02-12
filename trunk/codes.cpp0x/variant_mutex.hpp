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

namespace more { 

    namespace detail 
    {
        // template metafunction that return the max sizeof a list of types.
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
    }
     
    // Variant mutex for Lockable object. 
    // Requirement 30.2.5.3 Draft, Standard for C++ Programming Language

    template <typename ...Ts>
    class variant_mutex
    {
        public:
            variant_mutex(unsigned int n)
            : m_type(n)
            {
                if (n >= sizeof...(Ts))
                    throw std::runtime_error("variant_mutex: bad index");                

                apply<Ts...>::ctor(m_storage, m_type);
            }

            ~variant_mutex()
            {
                apply<Ts...>::dtor(m_storage, m_type);
            }

            void lock()
            {
                apply<Ts...>::lock(m_storage, m_type);
            }

            void unlock()
            {
                apply<Ts...>::unlock(m_storage, m_type);
            }

            bool try_lock()
            {
                return apply<Ts...>::try_lock(m_storage, m_type);
            }

        private:

        const unsigned int  m_type;
        char m_storage[ detail::max_sizeof<Ts...>::value ];
        
        template <typename ...Tp> struct apply;
        template <typename T, typename ...Tp>
        struct apply<T, Tp...> 
        {
            static void ctor(char *s, int type, int t = 0)
            {
                if (t == type) { new(s)T; return; }
                apply<Tp...>::ctor(s, type, t+1);    
            }

            static void dtor(char *s, int type, int t = 0)
            {
                if (t == type) { reinterpret_cast<T *>(s)->~T(); return; }
                apply<Tp...>::dtor(s, type, t+1);
            }

            static void lock(char *s, int type, int t = 0)
            {
                if (t == type) {  reinterpret_cast<T *>(s)->lock(); return; }
                apply<Tp...>::lock(s, type, t+1);
            }

            static void unlock(char *s, int type, int t = 0)
            {
                if (t == type) {  reinterpret_cast<T *>(s)->unlock(); return; }
                apply<Tp...>::unlock(s, type, t+1);
            }

            static bool try_lock(char *s, int type, int t = 0)
            {
                if (t == type) {  return reinterpret_cast<T *>(s)->try_lock(); }
                return apply<Tp...>::try_lock(s, type, t+1);
            }

        };
        template <typename T>
        struct apply<T>
        {
            static void ctor(char *s, int, int = 0)
            {
                new(s)T; 
            }
            static void dtor(char *s, int, int = 0)
            {
                reinterpret_cast<T *>(s)->~T(); 
            }
            
            static void lock(char *s, int, int = 0)
            {
                reinterpret_cast<T *>(s)->lock(); 
            }

            static void unlock(char *s, int, int = 0)
            {
                reinterpret_cast<T *>(s)->unlock(); 
            }

            static bool try_lock(char *s, int, int = 0)
            {
                return reinterpret_cast<T *>(s)->try_lock(); 
            }
        };
    };

} // namespace more

#endif /* _VARIANT_MUTEX_HPP_ */
