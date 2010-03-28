/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 *
 */

#ifndef ATOMICITY_HH
#define ATOMICITY_HH

#include <tr1_memory.hh>

#ifdef BOOST_HAS_THREADS
#include <boost/thread.hpp>
#endif

namespace more { namespace atomicity {

    struct null {};

    /// @brief  A template Scoped lock idiom (inspired to that of gnu_cxx).
    // Acquire the mutex here with a constructor call, then release with
    // the destructor call in accordance with RAII style.

    template <class M>
    class __scoped_lock
    {
    public:
        typedef M __mutex_type;

    private:
        __mutex_type& _M_device;

        __scoped_lock(const __scoped_lock&);
        __scoped_lock& operator=(const __scoped_lock&);

    public:
        explicit __scoped_lock(__mutex_type& __name) : _M_device(__name)
        { _M_device.lock(); }

        ~__scoped_lock() 
        { _M_device.unlock(); }
    };


    struct NONE {
        typedef null mutex;
        typedef null scoped_lock;
    };

    struct BOOST {
#ifdef BOOST_HAS_THREADS
        typedef boost::mutex mutex;
        typedef boost::mutex::scoped_lock scoped_lock;
#endif
    };

    struct BOOST_RECURSIVE {
#ifdef BOOST_HAS_THREADS
        typedef boost::recursive_mutex mutex;
        typedef boost::recursive_mutex::scoped_lock scoped_lock;
#endif
    };

    template <int, int> struct gnu_cxx;
    template <>
    struct gnu_cxx<4,0> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 0 && !defined(__INTEL_COMPILER) 
        typedef __gnu_cxx::mutex_type mutex;
        typedef __gnu_cxx::lock scoped_lock;
#endif
    };

    template <>
    struct gnu_cxx<4,1> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 1 && !defined(__INTEL_COMPILER)
        typedef __gnu_cxx::mutex_type mutex;
        typedef __gnu_cxx::lock scoped_lock;
#endif
    };

    template <>
    struct gnu_cxx<4,2> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 2  && !defined(__INTEL_COMPILER)
        typedef __gnu_cxx::__mutex mutex;
        typedef __gnu_cxx::__scoped_lock scoped_lock;
#endif
    };
    template <>
    struct gnu_cxx<4,3> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 3  && !defined(__INTEL_COMPILER)
        typedef __gnu_cxx::__mutex mutex;
        typedef __gnu_cxx::__scoped_lock scoped_lock;
#endif
    };

    template <>
    struct gnu_cxx<4,4> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 4  && !defined(__INTEL_COMPILER)
        typedef __gnu_cxx::__mutex mutex;
        typedef __gnu_cxx::__scoped_lock scoped_lock;
#endif
    };

    template <int, int> struct gnu_cxx_recursive;
    template <>
    struct gnu_cxx_recursive<4,2> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 2  && !defined(__INTEL_COMPILER)
        typedef __gnu_cxx::__recursive_mutex mutex;
        typedef atomicity::__scoped_lock<mutex> scoped_lock; // not yet sopported in gnu_cxx 
#endif
    };
    template <>
    struct gnu_cxx_recursive<4,3> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 3  && !defined(__INTEL_COMPILER)
        typedef __gnu_cxx::__recursive_mutex mutex;
        typedef atomicity::__scoped_lock<mutex> scoped_lock; // not yet supported in gnu_cxx
#endif
    };
    template <>
    struct gnu_cxx_recursive<4,4> {
#if   __GNUC__ == 4 &&  __GNUC_MINOR__ == 4  && !defined(__INTEL_COMPILER)
        typedef __gnu_cxx::__recursive_mutex mutex;
        typedef atomicity::__scoped_lock<mutex> scoped_lock; // not yet supported in gnu_cxx
#endif
    };

    typedef gnu_cxx<__GNUC__, __GNUC_MINOR__> GNU_CXX; 
    typedef gnu_cxx_recursive<__GNUC__, __GNUC_MINOR__> GNU_CXX_RECURSIVE; 

    /////////////////////////////////////////////////////////
    // default policy... atomicity::DEFAULT

#ifdef _REENTRANT

#if defined(__INTEL_COMPILER)

#   ifndef NDEBUG
#   warning "atomicity::DEFAULT set to BOOST"
#   endif

    typedef BOOST DEFAULT;
    typedef BOOST_RECURSIVE DEFAULT_RECURSIVE;
#else

#   ifndef NDEBUG
#   warning "atomicity::DEFAULT set to GNU_CXX"
#   endif

    typedef GNU_CXX DEFAULT;
    typedef GNU_CXX_RECURSIVE DEFAULT_RECURSIVE;
#endif

#else
#  ifndef NDEBUG
#  warning "atomicity::DEFAULT set to NONE"
#  endif
    typedef NONE DEFAULT;
    typedef NONE DEFAULT_RECURSIVE;
#endif

    /////////////////////////////////////////
    // empty-base omptimization (for mutex)

    template <typename POLICY>
    struct emptybase_mutex 
    {
        typename POLICY::mutex & mutex()
        { return _M_mutex; }

        static 
        typename POLICY::mutex & static_mutex()
        {
            static typename POLICY::mutex one; 
            return one; 
        }

    private:
        typename POLICY::mutex _M_mutex;
    };

    template <>
    struct emptybase_mutex<NONE>
    {
        null mutex()
        { return null(); }        
        
        static 
        null & static_mutex()
        {
            static null one; 
            return one; 
        }
    };
}}

#endif /* ATOMICITY_HH */
