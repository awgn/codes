/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _SHARED_PTR_H_
#define _SHARED_PTR_H_ 

#include <integral.hh>

#if defined(MORE_USE_QT_SHARED_PTR)
#include <QSharedPointer>
#   ifndef NDEBUG
#   warning using more::shared_ptr<> as QSharedPointer<>
#   endif
#elif defined(MORE_USE_BOOST_SHARED_PTR)
#include <boost/shared_ptr.hpp>
#   ifndef NDEBUG
#   warning using more::shared_ptr<> as boost::shared_ptr<> 
#   endif
#else // TR1
#include <tr1/memory>
#   ifndef NDEBUG
#   warning using more::shared_ptr<> as std::tr1::shared_ptr<> 
#   endif
#endif

namespace more { 

    template <typename Tp>
    struct shared_ptr
    {
#if defined(MORE_USE_QT_SHARED_PTR)

        struct type : public QSharedPointer<Tp> 
        {
            type()
            : QSharedPointer<Tp>()
            {}

            type(Tp *p)
            : QSharedPointer<Tp>(p)
            {}

            template <typename D>
            type(Tp *p, D d)
            : QSharedPointer<Tp>(p,d)
            {}

            ~type()
            {}

            // reset wrappers...

            void reset()
            {
                this->clear();
            }

            void reset(Tp * ptr)
            {
                type x(ptr);
                this->operator=(x);
            }

            template <typename D>
            void reset(Tp * ptr, D d)
            {
                type x(ptr,d);
                this->operator=(x);
            }

            // get wrapper:

            Tp * get() const
            {
                return this->data();
            }

        };

        typedef qt_type shared_ptr_type;

        struct is_tr1   : public false_type {};
        struct is_boost : public false_type {};
        struct is_qt    : public true_type {};

#elif defined(MORE_USE_BOOST_SHARED_PTR)

        typedef boost::shared_ptr<Tp> type;
        typedef boost_type shared_ptr_type;

        struct is_tr1   : public false_type {};
        struct is_boost : public true_type {};
        struct is_qt    : public false_type {};

#else // std::tr1

        typedef std::tr1::shared_ptr<Tp> type;
        typedef tr1_type shared_ptr_type;        
        
        struct is_tr1   : public true_type {};
        struct is_boost : public false_type {};
        struct is_qt    : public false_type {};
#endif
    };

} // namespace more 

#endif /* _SHARED_PTR_H_ */
