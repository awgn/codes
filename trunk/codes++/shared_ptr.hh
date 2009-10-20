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
#elif defined(MORE_USE_TR1_SHARED_PTR)
#include <tr1/memory>
#   ifndef NDEBUG
#   warning using more::shared_ptr<> as std::tr1::shared_ptr<> 
#   endif
#endif

namespace more { 

#if defined(MORE_USE_QT_SHARED_PTR)

    template <typename Tp>
    struct shared_ptr : public QSharedPointer<Tp>
    {
        typedef shared_ptr<Tp> type;

        shared_ptr()
        : QSharedPointer<Tp>()
        {}

        explicit shared_ptr(Tp *p)
        : QSharedPointer<Tp>(p)
        {}

        template <typename D>
        shared_ptr(Tp *p, D d)
        : QSharedPointer<Tp>(p,d)
        {}

        shared_ptr(const type &rhs)
        : QSharedPointer<Tp>(rhs)
        {}

        shared_ptr(const QSharedPointer<Tp> &rhs)
        : QSharedPointer<Tp>(rhs)
        {}

        ~shared_ptr()
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

        typedef qt_type shared_ptr_type;

        struct is_tr1   : public false_type {};
        struct is_boost : public false_type {};
        struct is_qt    : public true_type {};

    };

#elif defined(MORE_USE_BOOST_SHARED_PTR)
    template <typename Tp>
    struct shared_ptr
    {
 
        typedef boost::shared_ptr<Tp> type;
        typedef boost_type shared_ptr_type;

        struct is_tr1   : public false_type {};
        struct is_boost : public true_type {};
        struct is_qt    : public false_type {};
    };
#elif defined(MORE_USE_TR1_SHARED_PTR)

    template <typename Tp>
    struct shared_ptr 
    {
 
        typedef std::tr1::shared_ptr<Tp> type;
        typedef tr1_type shared_ptr_type;        
        
        struct is_tr1   : public true_type {};
        struct is_boost : public false_type {};
        struct is_qt    : public false_type {};
    };

#endif

#if defined(MORE_USE_QT_SHARED_PTR)

    template <class O, class Ty>
    static inline 
    typename more::shared_ptr<O> static_pointer_cast(const typename more::shared_ptr<Ty> &sp)
    {
        return typename more::shared_ptr<O>( static_cast<QSharedPointer<Ty> >(sp).template staticCast<O>() );    
    }

    template <class O, class Ty>
    static inline 
    typename more::shared_ptr<O> dynamic_pointer_cast(const typename more::shared_ptr<Ty> &sp)
    {
        return typename more::shared_ptr<O>( static_cast<QSharedPointer<Ty> >(sp).template dynamicCast<O>() );    
    }

    template <class O, class Ty>
    static inline 
    typename more::shared_ptr<O> const_pointer_cast(const typename more::shared_ptr<Ty> &sp)
    {
        return typename more::shared_ptr<O>( static_cast<QSharedPointer<Ty> >(sp).template constCast<O>() );    
    }

#elif defined(MORE_USE_BOOST_SHARED_PTR)

    template <class O, class Ty>
    static inline 
    boost::shared_ptr<O> static_pointer_cast(const boost::shared_ptr<Ty> &sp)
    {
        return boost::static_pointer_cast<O>(sp);    
    }

    template <class O, class Ty>
    static inline 
    boost::shared_ptr<O> dynamic_pointer_cast(const boost::shared_ptr<Ty> &sp)
    {
        return boost::dynamic_pointer_cast<O>(sp);    
    }

    template <class O, class Ty>
    static inline 
    boost::shared_ptr<O> const_pointer_cast(const boost::shared_ptr<Ty> &sp)
    {
        return boost::const_pointer_cast<O>(sp);    
    }

#elif defined(MORE_USE_TR1_SHARED_PTR)

    template <class O, class Ty>
    static inline 
    std::tr1::shared_ptr<O> static_pointer_cast(const std::tr1::shared_ptr<Ty> &sp)
    {
        return std::tr1::static_pointer_cast<O>(sp);    
    }

    template <class O, class Ty>
    static inline 
    std::tr1::shared_ptr<O> dynamic_pointer_cast(const std::tr1::shared_ptr<Ty> &sp)
    {
        return std::tr1::dynamic_pointer_cast<O>(sp);    
    }

    template <class O, class Ty>
    static inline 
    std::tr1::shared_ptr<O> const_pointer_cast(const std::tr1::shared_ptr<Ty> &sp)
    {
        return std::tr1::const_pointer_cast<O>(sp);    
    }

#endif

} // namespace more 

#endif /* _SHARED_PTR_H_ */
