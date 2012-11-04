/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _VARIANT_HPP_
#define _VARIANT_HPP_ 

#include <typeinfo>
#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

// Yet another boost tribute: the class variant.
//

namespace more { 

    namespace detail 
    {
        template <typename ...Ti>  struct max_sizeof;
        template <typename T0, typename ...Ti>
        struct max_sizeof<T0, Ti...>
        {
            enum { value = (sizeof(T0) > max_sizeof<Ti...>::value) ? static_cast<int>(sizeof(T0)) : max_sizeof<Ti...>::value };
        };  

        template <typename T0>
        struct max_sizeof<T0>
        {
            enum { value = sizeof(T0) };
        };  

        ///////////////////////////////////////////// type index 

        template <int N, typename T, typename ...Ti> struct __index_of;

        template <int N, typename T, typename T0, typename ...Ti>
        struct __index_of<N,T,T0,Ti...>
        {
            enum { value = std::is_same<T,T0>::value ? N : __index_of<N+1, T, Ti...>::value };
        };

        template <int N, typename T, typename T0>
        struct __index_of<N,T,T0>
        {
            enum { value = std::is_same<T,T0>::value ? N : -1 };
        };

        template <typename T, typename ...Ti> struct index_of
        {
            enum { value = __index_of<0, T, Ti...>::value };    
        };

        ////////////////////////////////////////////// get n-th type

        template <int i, int N, typename ...Ti> struct __get_type;
        template <int i, int N, typename T0, typename ...Ti>
        struct __get_type<i,N, T0, Ti...>
        {
            typedef typename __get_type<i+1, N, Ti...>::type type;
        };
        template <int N, typename T0, typename ...Ti>
        struct __get_type<N,N, T0, Ti...>
        {
            typedef T0 type;
        };
        template <int N, typename ...Ti> struct get_type
        {
            typedef typename __get_type<0,N, Ti...>::type type;
        };  
 
    }   // namespace detail

    template <typename ...Ti>  
    class variant {

    public:
        template <typename CharT, typename Traits>
        friend std::basic_ostream<CharT,Traits> &
        operator<<(std::basic_ostream<CharT,Traits> &out, const more::variant<Ti...> & var)
        {
            variant<Ti...>::printon<Ti...>::apply(out, var);
            return out; 
        }

    public:
        variant()
        : m_type(0)
        {
            ctor<Ti...>::apply(m_storage, 0);
        }

        variant(const variant &rhs)
        : m_type(rhs.m_type)
        {
            copyctor<Ti...>::apply(rhs.m_storage, m_storage, m_type); 
        }
        
        variant(const variant &&rhs)
        : m_type(rhs.m_type)
        {
            rhs.m_mtype = -1;
            movector<Ti...>::apply(rhs.m_storage, m_storage, m_type); 
        }

        variant& operator=(variant rhs)
        {    
            rhs.swap(*this);
            return *this;
        }
 
        ~variant()
        {
            if (m_type != -1)
                dtor<Ti...>::apply(m_storage, m_type);
        }
        
        void swap(variant &rhs)
        {
            std::swap_ranges(m_storage, m_storage+_S_storage_size, rhs.m_storage);
            std::swap(m_type, rhs.m_type);
        }

        template <typename T>
        variant(const T &value)
        : m_type(0)
        {
            ctor<Ti...>::apply(m_storage, 0);
            try 
            {
                this->store(value);
            }
            catch(...)
            {
                dtor<Ti...>::apply(m_storage, 0);
                throw;
            }
        }

        template <typename T>
        variant &operator=(const T& rhs)
        {
            this->store(rhs);
            return *this;
        }
 
        bool 
        empty() const 
        {
            return false;   // never empty
        }

        int
        which() const
        { return m_type; }

        int
        storage_size() const
        {
            return sizeof(m_storage)/sizeof(m_storage[0]);
        }

        const std::type_info &
        type() const
        {
            return __type<Ti...>::get(m_type);
        }

        /////////////////////

        template <typename T>
        T & get()
        {
            if ( detail::index_of<T,Ti...>::value != m_type )
                throw std::bad_cast();

            return *reinterpret_cast<T *>(m_storage);
        }

        template <typename T>
        const T & get() const
        {
            if ( detail::index_of<T,Ti...>::value != m_type )
                throw std::bad_cast();

            return *reinterpret_cast<const T *>(m_storage);
        }

        template <typename F>
        void apply_visitor(F cw)
        {
            visitor<Ti...>::apply(cw, *this);
        }

        template <typename V>
        void store(const V &value)
        {    
            static_assert(detail::index_of<V, Ti...>::value != -1, "type not in variant");
            char tmp[_S_storage_size];
                   
            // try to construct the new value in a temporary storage
            //
            new (tmp) V(value);
            
            // destroy the object in the m_storage (as m_type) ...
            //
            dtor<Ti...>::apply(m_storage, m_type);  
 
            // copy the temporary storage to m_storage
            //
            std::copy(tmp, tmp + _S_storage_size, m_storage);
            m_type = detail::index_of<V, Ti...>::value;
        }

    private:

        template <typename ... Tp> struct __type;
        template <typename T, typename ... Tp> 
        struct __type<T, Tp...>
        {
            static const std::type_info &
            get(int tp, int n = 0)
            {
                if (tp == n)
                    return typeid(T);
                else
                    return __type<Tp...>::get(tp, n+1);
            }
        };
        template <typename T>
        struct __type<T>
        {
            static const std::type_info &
            get(int tp, int n = 0)
            {
                if (tp == n)
                    return typeid(T);
                else
                    throw std::runtime_error("__type: internal error");
            }
        };
 
        ////////
        
        template <typename ...Tp> struct ctor;
        template <typename T, typename ...Tp>
        struct ctor<T, Tp...> 
        {
            static void apply(char *storage, int type, int n = 0)
            {
                if (n == type) 
                {
                    new(storage)T;
                    return;
                }
                ctor<Tp...>::apply(storage, type, n+1);    
            }
        };
        template <typename T>
        struct ctor<T>
        {
            static void apply(char *storage, int type, int n = 0)
            {
                if (n == type) 
                {
                    new(storage)T;
                    return;
                }
                throw std::runtime_error("internal error");
            }
        };
 
        
        template <typename ...Tp> struct dtor;
        template <typename T, typename ...Tp>
        struct dtor<T, Tp...> 
        {
            static void apply(char *storage, int type, int n = 0)
            {
                if (n == type) 
                {
                    reinterpret_cast<T *>(storage)->~T();
                    return;
                }
                dtor<Tp...>::apply(storage, type, n+1);    
            }
        };
        template <typename T>
        struct dtor<T>
        {
            static void apply(char *storage, int type, int n = 0)
            {
                if (n == type) 
                {
                    reinterpret_cast<T *>(storage)->~T();
                    return;
                }
                throw std::runtime_error("internal error");
            }
        };


        template <typename ...Tp> struct copyctor;                   
        template <typename T, typename ...Tp>
        struct copyctor<T, Tp...> 
        {
            static void apply(const char *from, char *to, int type, int n = 0)
            {
                if (n == type) 
                {
                    new (to) T(*reinterpret_cast<const T *>(from));
                    return;
                }
                copyctor<Tp...>::apply(from, to, type, n+1);    
            }
        };
        template <typename T>
        struct copyctor<T>
        {
            static void apply(const char *from, char *to, int type, int n = 0)
            {
                if (n == type) 
                {
                    new (to) T(*reinterpret_cast<const T *>(from));
                    return;
                }
                throw std::runtime_error("internal error");
            }
        };
 
        
        template <typename ...Tp> struct movector;                   
        template <typename T, typename ...Tp>
        struct movector<T, Tp...> 
        {
            static void apply(const char *from, char *to, int type, int n = 0)
            {
                if (n == type) 
                {
                    new (to) T(std::move(*reinterpret_cast<const T *>(from)));
                    return;
                }
                movector<Tp...>::apply(from, to, type, n+1);    
            }
        };
        template <typename T>
        struct movector<T>
        {
            static void apply(const char *from, char *to, int type, int n = 0)
            {
                if (n == type) 
                {
                    new (to) T(std::move(*reinterpret_cast<const T *>(from)));
                    return;
                }
                throw std::runtime_error("internal error");
            }
        };

        template <typename ...Tp> struct printon;
        template <typename T, typename ...Tp>
        struct printon<T, Tp...> 
        {
            template <typename CharT, typename Traits, typename V>
            static void apply(std::basic_ostream<CharT, Traits> &out, const V &var, int n = 0)
            {
                if (n == var.m_type) 
                {
                    out << var.template get<T>();
                    return;
                }
                printon<Tp...>::apply(out, var, n+1);    
            }
        };
        template <typename T>
        struct printon<T>
        {
            template <typename CharT, typename Traits, typename V>
            static void apply(std::basic_ostream<CharT, Traits> &out, const V &var, int n = 0)
            {
                if (n == var.m_type) 
                {
                    out << var.template get<T>();
                    return;
                }
                throw std::runtime_error("internal error");
            }
        };
      
       
        template <typename ...Tp> struct visitor;
        template <typename T, typename ...Tp>
        struct visitor<T, Tp...> 
        {
            template <typename F, typename V>
            static void apply(F cw, const V &var, int n = 0)
            {
                if (n == var.m_type) 
                {
                    cw(var.template get<T>());
                    return;
                }
                visitor<Tp...>::apply(cw, var, n+1);    
            }
        };
        template <typename T>
        struct visitor<T>
        {
            template <typename F, typename V>
            static void apply(F cw, const V &var, int n = 0)
            {
                if (n == var.m_type) 
                {
                    cw(var.template get<T>());
                    return;
                }
                throw std::runtime_error("internal error");
            }
        };
 
        static const int _S_storage_size = detail::max_sizeof<Ti...>::value;
        
        int  m_type;
        char m_storage[_S_storage_size];
    };

} // namespace more

#endif /* _VARIANT_HPP_ */
