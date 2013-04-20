/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _ORACLE_HH_
#define _ORACLE_HH_ 

#include <cxxabi.h>
#include <colorful.hh>

#ifdef _REENTRANT
#include <atomicity-policy.hh>
#include <atomic.hh>
#endif
#include <tr1/memory>       

#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <algorithm>
#include <iterator>
#include <set>

#define oracle_std_swap(CLASS)  \
namespace std   \
{   \
    template <> \
    void swap(CLASS &x, CLASS &y)   \
    {   \
        x.swap(y);  \
    }   \
}

#define __oracle_tp_check(n)   \
    if (oracle_helper::throw_point::is_enabled()) \
        throw std::runtime_error(std::string("oracle<").append(name()).append(">"));

#define __oracle_header(c, var, color)  \
    std::setw(2) << oracle_helper::throw_point::counter()++ << ':' << \
    ' ' << c << std::setw(2) << var << ' ' << *this << ' ' << \
        more::colorful< TYPELIST(more::ecma::bold,more::ecma::color) >() << __PRETTY_FUNCTION__ << \
        more::colorful< TYPELIST(more::ecma::reset) >()

#define __oracle_header_static(c, var, color)  \
    std::setw(2) << oracle_helper::throw_point::counter()++ << ':' << \
    ' ' << c << std::setw(2) << var << ' ' << \
        more::colorful< TYPELIST(more::ecma::bold,more::ecma::color) >() << __PRETTY_FUNCTION__ << \
        more::colorful< TYPELIST(more::ecma::reset) >()

namespace more { 

    namespace oracle_helper 
    {
        static inline
        std::string 
        demangle(const char *name)
        {
#ifdef _REENTRANT
            static atomicity::GNU_CXX::mutex s_mutex;
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            int status;
            std::tr1::shared_ptr<char> ret(abi::__cxa_demangle(name,0,0, &status), ::free);                    
            if (status < 0)
                throw std::runtime_error("__cxa_demangle");
            
            return std::string(ret.get());
        }

        struct throw_point 
        {

#ifdef _REENTRANT
            static more::atomic<int> & 
#else
            static int & 
#endif
            counter()
            {
#ifdef _REENTRANT
                static more::atomic<int> one(0);
#else
                static int one;
#endif
                return one;
            }

            static const std::set<int> &
            set()
            {
                static tp one;  // protected by gnu guard
                return one.get();
            }

            static bool
            is_enabled()
            {
                return set().find(counter()) != set().end();
            }   

        private:

            struct tp 
            {
                tp()    
                : m_set()
                {
                    // load the throw points...
                    if ( char * p = ::getenv("oracle_tp")) {
                        std::stringstream in(p);
                        for(int n; in >> n ;) {
                            m_set.insert(n);
                        }
                    }
                    
                    if (m_set.empty())
                        return;

                    std::cout << more::colorful< TYPELIST(more::ecma::bold)>() << "oracle throw points: " <<
                                 more::colorful< TYPELIST(more::ecma::reset) >();
                    std::copy(m_set.begin(), m_set.end(), std::ostream_iterator<int>(std::cout, " "));
                    std::cout << std::endl;
                } 

                ~tp()
                {}

                const
                std::set<int> &
                get() const
                {
                    return m_set;
                }

            private:
                std::set<int> m_set;
            };

        };

    }

    template <typename E>
    class oracle
    {
    public:
        typedef oracle<E>   oracle_base;
        
        // normal new/delete
        //

        static void * operator new(std::size_t size) throw (std::bad_alloc)
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header_static('N', ++s_new, fg_blue) << std::endl;
            return ::operator new(size);
        }

        static void operator delete(void *mem) throw()
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            std::cerr << __oracle_header_static('D', ++s_del, fg_red) << std::endl;
            ::operator delete(mem);
        }

        // normal new[]/delete[]
        //

         static void * operator new[](std::size_t size) throw (std::bad_alloc)
         {
 #ifdef _REENTRANT
             atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
 #endif
             __oracle_tp_check();
             std::cerr << __oracle_header_static('N', ++s_new, fg_blue) << std::endl;
             return ::operator new(size);
         }
 
         static void operator delete[](void *mem) throw()
         {
 #ifdef _REENTRANT
             atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
 #endif
             std::cerr << __oracle_header_static('D', ++s_del, fg_red) << std::endl;
             ::operator delete(mem);
         }

        // placement new/delete
        //

         void * operator new(std::size_t size, char *ptr) throw()
         {
 #ifdef _REENTRANT
             atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
 #endif
             __oracle_tp_check();
             std::cerr << __oracle_header_static('N', ++s_new, fg_blue) << std::endl;
             return ::operator new(size,ptr);
         }
 
        void operator delete(void *mem, char *ptr) throw()
         {
 #ifdef _REENTRANT
             atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
 #endif
             std::cerr << __oracle_header_static('D', ++s_del, fg_red) << std::endl;
             ::operator delete(mem,ptr);
         }

        // placement new[]/delete[]
        //

         void * operator new[](std::size_t size, char *ptr) throw()
         {
 #ifdef _REENTRANT
             atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
 #endif
             __oracle_tp_check();
             std::cerr << __oracle_header_static('N', ++s_new, fg_blue) << std::endl;
             return ::operator new(size,ptr);
         }
 
        void operator delete[](void *mem, char *ptr) throw()
         {
 #ifdef _REENTRANT
             atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
 #endif
             std::cerr << __oracle_header_static('D', ++s_del, fg_red) << std::endl;
             ::operator delete(mem,ptr);
         }

        // constructors, copy constructor, operator= and destructor
        //

        oracle()
        : m_value(oracle_helper::throw_point::counter())
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('+', ++s_ctor, fg_blue) << std::endl;
            s_set.insert(this);
        }

        oracle(const oracle &rhs)
        : m_value(rhs.m_value)
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('+', ++s_ctor, fg_cyan) << std::endl;
            s_set.insert(this);
        } 

        template <typename T1>
        oracle(const T1 &)
        : m_value(oracle_helper::throw_point::counter())
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('+', ++s_ctor, fg_blue) << std::endl;
            s_set.insert(this);
        } 

        template <typename T1, typename T2>
        oracle(const T1 &, const T2 &)
        : m_value(oracle_helper::throw_point::counter())
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('+', ++s_ctor, fg_blue) << std::endl;
            s_set.insert(this);
        }

        template <typename T1, typename T2, typename T3>
        oracle(const T1 &, const T2 &, const T3 &)
        : m_value(oracle_helper::throw_point::counter())
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('+', ++s_ctor, fg_blue) << std::endl;
            s_set.insert(this);
        }

        template <typename T1, typename T2, typename T3, typename T4>
        oracle(const T1 &, const T2 &, const T3 &, const T4 &)
        : m_value(oracle_helper::throw_point::counter())
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('+', ++s_ctor, fg_blue) << std::endl;
            s_set.insert(this);
        }

        template <typename T1, typename T2, typename T3, typename T4, typename T5>
        oracle(const T1 &, const T2 &, const T3 &, const T4 &, const T5 &)
        : m_value(oracle_helper::throw_point::counter())
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('+', ++s_ctor, fg_blue) << std::endl;
            s_set.insert(this);
        }        

        oracle &
        operator=(const oracle &rhs)
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            __oracle_tp_check();
            std::cerr << __oracle_header('=', ++s_opeq, fg_green) << std::endl;
            m_value = rhs.m_value;
            return *this;
        }

        ~oracle()
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            std::cerr << __oracle_header('-', ++s_dtor, fg_red) << std::endl;
            s_set.erase(this);
        }

        void swap(oracle &rhs)
        {
#ifdef _REENTRANT
            atomicity::GNU_CXX::scoped_lock _L_(s_mutex);
#endif
            std::cerr << __oracle_header( 'x', ++s_swap, fg_magenta) << std::endl;
            std::swap(m_value, rhs.m_value);
        }

        // for STL algorithms...
        //

        bool 
        operator<(const oracle &rhs) const
        {
            return m_value < rhs.m_value;
        }

        // other static methods
        //

        static int
        ctors()
        {
            return s_ctor;
        }
        static int
        dtors()
        {
            return s_dtor;
        }
        static int
        opeqs()
        {
            return s_opeq;
        }
        static int
        swaps()
        {
            return s_swap;
        }
        static int
        op_new()
        {
            return s_new;
        }
        static int
        op_del()
        {
            return s_del;
        }

        static
        std::string
        name()
        {
            return oracle_helper::demangle(typeid(E).name());
        }

        template <typename CharT, typename Traits>
        friend inline std::basic_ostream<CharT,Traits> &
        operator<< (std::basic_ostream<CharT, Traits> &out, const oracle &s)
        {
            out << '(' << std::setw(3) << s.m_value << ' ' << std::setw(10) << &s << ')' << std::dec;
            return out;
        }

        static std::set<oracle *> s_set;

    private:
        unsigned int    m_value;

#ifdef _REENTRANT
        static atomicity::GNU_CXX::mutex s_mutex;
#endif
        static int s_ctor;
        static int s_dtor;
        static int s_opeq;
        static int s_swap;
        static int s_new;
        static int s_del;
    };

    template<typename E>
    static void swap(oracle<E> &x, oracle<E> &y)
    {
        x.swap(y);
    }

#ifdef _REENTRANT
    template <typename E>
    atomicity::GNU_CXX::mutex oracle<E>::s_mutex;
#endif

    template <typename E>
    int oracle<E>::s_ctor(0);

    template <typename E>
    int oracle<E>::s_dtor(0);

    template <typename E>
    int oracle<E>::s_opeq(0);

    template <typename E>
    int oracle<E>::s_swap(0);

    template <typename E>
    int oracle<E>::s_new(0);

    template <typename E>
    int oracle<E>::s_del(0);

    template <typename E>
    std::set<oracle<E> *> oracle<E>::s_set;

    struct oracle_set_dump
    {
        explicit oracle_set_dump(std::ostream &o)
        : out(o)
        {}

        template <typename T>
        void operator()(oracle<T> *ptr) const
        {
            out << *ptr;
        }

        std::ostream &out;
    };

    template <typename E>
    struct oracle_trace 
    {
        void 
        printon(std::ostream &out) const
        {
            out << more::colorful< TYPELIST(more::ecma::bold)>() << 
                  "oracle<" << oracle_helper::demangle(typeid(E).name()) << "> stats" <<
                   more::colorful< TYPELIST(more::ecma::reset)>() << 
                   " { ctors:" << oracle<E>::ctors() << 
                   " dtors:" << oracle<E>::dtors() << 
                   " oper=:" << oracle<E>::opeqs() << 
                   " swaps:" << oracle<E>::swaps() <<
                   " new:" << oracle<E>::op_new() <<
                   " del:" << oracle<E>::op_del() <<
                   " } "; 

            std::for_each( oracle<E>::s_set.begin(), oracle<E>::s_set.end(), oracle_set_dump(std::cout) ); 
        }

        static oracle_trace & 
        enable()
        {
            static oracle_trace<E> one;
            return one;
        }
    private:

        oracle_trace()
        {} 
        
        ~oracle_trace()
        {
            std::cout << oracle_trace::enable() << std::endl; 
        }
        
    };

    template <typename CharT, typename Traits, typename E>
    inline std::basic_ostream<CharT,Traits> &
    operator<< (std::basic_ostream<CharT, Traits> &out, const oracle_trace<E> &s)
    {
        s.printon(out); 
        return out;
    }

} // namespace more

#endif /* _ORACLE_HH_ */
