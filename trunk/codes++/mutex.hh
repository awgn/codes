/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _MUTEX_HH_
#define _MUTEX_HH_ 

#include <integral.hh>

#if defined (MORE_USE_BOOST_MUTEX)
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#   ifndef NDEBUG
#   warning using more::mutex as boost::mutex 
#   endif
#elif defined(MORE_USE_QT_MUTEX)
#include <QMutex>
#include <QMutexLocker>
#   ifndef NDEBUG
#   warning using more::mutex as QMutex 
#   endif
#elif defined (MORE_USE_GNU_MUTEX)
#include <tr1/memory>
#   ifndef NDEBUG
#   warning using more::mutex as gnu_cxx::mutex 
#   endif
#else
#error MORE_USE_XXX_MUTEX not defined
#endif

namespace more { 

    namespace {   

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

    }

    struct mutex 
    {
#if defined(MORE_USE_BOOST_MUTEX)
        typedef boost::mutex type;
        typedef boost::mutex::scoped_lock scoped_lock;

        struct is_boost : public true_type {};
        struct is_qt    : public false_type {};
        struct is_gnu   : public false_type {};

        struct has_try_lock : public true_type {};

#elif defined(MORE_USE_QT_MUTEX)
        struct type : public QMutex
        {
            type() : QMutex()
            {}

            ~type()
            {}

            bool try_lock()
            {
                return this->tryLock();
            }
        };

        struct scoped_lock : public QMutexLocker
        {
            scoped_lock(QMutex &m) : QMutexLocker(&m)
            {}

            ~scoped_lock()
            {}
        };

        struct is_boost : public false_type {};
        struct is_qt    : public true_type {};
        struct is_gnu   : public false_type {};

        struct has_try_lock : public true_type {};

#elif defined(MORE_USE_GNU_MUTEX)
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 0  
        typedef __gnu_cxx::mutex_type type;
        typedef __gnu_cxx::lock scoped_lock;
    #endif
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 1   
        typedef __gnu_cxx::mutex_type type;
        typedef __gnu_cxx::lock scoped_lock;
    #endif
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 2  
        typedef __gnu_cxx::__mutex type;
        typedef __gnu_cxx::__scoped_lock scoped_lock;
    #endif
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 3  
        typedef __gnu_cxx::__mutex type;
        typedef __gnu_cxx::__scoped_lock scoped_lock;
    #endif

        struct is_boost : public false_type {};
        struct is_qt    : public false_type {};
        struct is_gnu   : public true_type {};

        struct has_try_lock : public false_type {};
#endif
    };

    struct recursive_mutex 
    {
#if defined(MORE_USE_BOOST_MUTEX)
        typedef boost::recursive_mutex type;
        typedef boost::recursive_mutex::scoped_lock scoped_lock;

        struct is_boost : public true_type {};
        struct is_qt    : public false_type {};
        struct is_gnu   : public false_type {};

        struct has_try_lock : public true_type {};

#elif defined(MORE_USE_QT_MUTEX)
        struct type : public QMutex
        {
            type() : QMutex(QMutex::Recursive)
            {}

            ~type()
            {}

            bool try_lock()
            {
                return this->tryLock();
            }
        };

        struct scoped_lock : public QMutexLocker
        {
            scoped_lock(QMutex &m) : QMutexLocker(&m)
            {}

            ~scoped_lock()
            {}
        };

        struct is_boost : public false_type {};
        struct is_qt    : public true_type {};
        struct is_gnu   : public false_type {};

        struct has_try_lock : public true_type {};

#elif defined(MORE_USE_GNU_MUTEX)
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 0  
    #error recursive_mutex not supported 
    #endif
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 1   
    #error recursive_mutex not supported 
    #endif
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 2  
        typedef __gnu_cxx::__recursive_mutex type;
        typedef __scoped_lock<type> scoped_lock;
    #endif
    #if __GNUC__ == 4 && __GNUC_MINOR__ == 3  
        typedef __gnu_cxx::__recursive_mutex type;
        typedef __scoped_lock<type> scoped_lock;
    #endif

        struct is_boost : public false_type {};
        struct is_qt    : public false_type {};
        struct is_gnu   : public true_type {};

        struct has_try_lock : public false_type {};
#endif
    };

} // namespace more

#endif /* _MUTEX_HH_ */
