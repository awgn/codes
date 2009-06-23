#ifndef _POLYMORPHIC_CAST_HH_
#define _POLYMORPHIC_CAST_HH_ 

#include <tr1/memory>
#include <cassert>
#include <stdexcept>

namespace more { 

    ///////////////////////////////////////////////////////////////////
    // polymorphic_cast (better dynamic_cast for pointers) ala boost...

    template <typename R, typename T>
    inline R polymorphic_cast(T *p)
    {
        R ret = dynamic_cast<R>(p);
        if ( ret == 0 ) throw std::bad_cast();
        return ret; 
    }

    /////////////////////////////////////
    // polymorphic_downcast ala boost...

    template <typename R, typename T>
    inline R polymorphic_downcast(T *p)
    {
        assert( dynamic_cast<R>(p) == p );
        return static_cast<R>(p);
    }

    /////////////////////////////////////////////////
    // polymorphic_cast for std::tr1::shared_ptr

    template <typename R, typename T>
    inline std::tr1::shared_ptr<R> polymorphic_pointer_cast(const std::tr1::shared_ptr<T> & p)
    {
        std::tr1::shared_ptr<R> ret = std::tr1::dynamic_pointer_cast<R>(p);
        if (!ret) throw std::bad_cast();
        return ret;
    }

    /////////////////////////////////////////////////
    // polymorphic_downcast for std::tr1::shared_ptr

    template <typename R, typename T>
    inline std::tr1::shared_ptr<R> polymorphic_pointer_downcast(const std::tr1::shared_ptr<T> & p)
    {
        assert( std::tr1::dynamic_pointer_cast<R>(p).get() );
        return std::tr1::static_pointer_cast<R>(p);
    }


} // namespace more

#endif /* _POLYMORPHIC_CAST_HH_ */
