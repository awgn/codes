/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MOVABLE_HH_
#define _MOVABLE_HH_ 

#include <tr1/functional>

namespace more { 

    template <typename T>
    class movable
    {
    public:
        explicit movable(T &from)
        : _M_from(from)
        {}
       
        ~movable()
        {}

        T &
        get()
        {
            return _M_from;
        }

    private:
        T & _M_from;

        movable & operator=(const movable &x);
    };

    template <typename T>
    inline T move(T &x)
    {
        return T(more::movable<T>(x));  // this works by means of RVO!
    }


} // namespace more

#endif /* _MOVABLE_HH_ */
