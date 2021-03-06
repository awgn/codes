/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _CURRYING_HPP_
#define _CURRYING_HPP_

#include <utility>
#include <type_traits>

namespace more {

    template <typename C, typename T>
    struct _curry_type
    {
        template <typename Ti>
        _curry_type(C call, Ti && a)
        : call_(std::move(call))
        , a_(std::forward<Ti>(a))
        {}

        _curry_type(_curry_type const &) = default;

        _curry_type(_curry_type &&) = default;

        ~_curry_type() = default;

        template <typename ... Ti>
        auto operator()(Ti && ... as)
        -> decltype(std::declval<C>()
                    (std::forward<T>(std::declval<T>()),
                     std::forward<Ti>(std::declval<Ti>())...))
        {
            return call_(std::forward<T>(a_), std::forward<Ti>(as)...);
        }

    private:

        C call_;
        T a_;
    };


    /// curry
    ///

    template <typename C, typename T>
    _curry_type<C, T>
    curry(C call, T && a)
    {
        return _curry_type<C, T>(std::move(call), std::forward<T>(a));
    }

    template <typename C, typename T, typename ... Ts>
    auto curry(C call, T && a, Ts && ... as)
    -> decltype(curry(_curry_type<C, T>(std::move(call), std::forward<T>(a)), std::forward<Ts>(as)...))
    {
        return curry(curry(std::move(call), std::forward<T>(a)), std::forward<Ts>(as)...);
    }

    /// closure
    ///

    template <typename T>
    struct closure_type
    {
        typedef typename std::add_const<typename std::remove_reference<T>::type>::type type;
    };

    template <typename C, typename T>
    _curry_type<C, typename closure_type<T>::type>
    closure(C call, T && a)
    {
        return _curry_type<C, typename closure_type<T>::type>(call, std::forward<T>(a));
    }

    template <typename C, typename T, typename ... Ts>
    auto closure(C call, T && a, Ts && ... as)
    -> decltype(closure(_curry_type<C, typename closure_type<T>::type>(call, std::forward<T>(a)), std::forward<Ts>(as)...))
    {
        return closure(closure(call, std::forward<T>(a)), std::forward<Ts>(as)...);
    }

} // namespace more

#endif /* _CURRYING_HPP_ */
