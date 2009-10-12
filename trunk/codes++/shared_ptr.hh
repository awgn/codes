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

    namespace {     // TR1 helper classes [4.3]

        template <typename Tp, Tp _v>
        struct integral_constant
        {
            typedef Tp  value_type;
            typedef integral_constant<Tp,_v> type;

            static const Tp     value = _v;
        }; 

        typedef integral_constant<bool, true > true_type;
        typedef integral_constant<bool, false> false_type;
    } 

    // shared_ptr traits:

    struct std_type {}; 

    struct qt_type {};
    struct tr1_type   : public std_type {};
    struct boost_type : public std_type {};

    template <typename Tp>
    struct shared_ptr
    {
#if defined(MORE_USE_QT_SHARED_PTR)

        typedef QSharedPointer<Tp> type;
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
