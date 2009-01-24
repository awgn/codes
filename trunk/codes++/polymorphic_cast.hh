#ifndef _POLYMORPHIC_CAST_HH_
#define _POLYMORPHIC_CAST_HH_ 

#include <tr1/memory>
#include <cassert>

namespace more { 

    /////////////////////////////////////
    // polymorphic_downcast ala boost...

    template <typename R, typename T>
    inline R polymorphic_downcast(T p)
    {
        assert( dynamic_cast<R>(p) == p );
        return static_cast<R>(p);
    }

    /////////////////////////////////////////////////
    // polymorphic_downcast for std::tr1::shared_ptr

    template <typename R, typename T>
    inline std::tr1::shared_ptr<R> polymorphic_pointer_downcast(std::tr1::shared_ptr<T> & p)
    {
        assert( std::tr1::dynamic_pointer_cast<R>(p).get() == p.get() );
        return std::tr1::static_pointer_cast<R>(p);
    }


} // namespace more

#endif /* _POLYMORPHIC_CAST_HH_ */
