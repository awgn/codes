/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _TUPLE_EXT_HPP_
#define _TUPLE_EXT_HPP_

#include <cstddef>
#include <tuple>
#include <algorithm>

namespace more {

    template <typename Tp>
    struct remove_reference_and_cv
    {
        typedef typename std::remove_cv<
                typename std::remove_reference<Tp>::type>::type type;
    };

    /// tuple_for_each

    // can't use simple recursion with variadic template functions
    // because there's no simple way to get the tail of a tuple
    //

    template <size_t N>
    struct tuple_
    {
        template <typename Fun, typename TupleT>
        static inline
        void for_each(TupleT &&tup, Fun fun)
        {
            fun(std::get<std::tuple_size<typename remove_reference_and_cv<TupleT>::type>::value - N>(std::forward<TupleT>(tup)));
            tuple_<N-1>::for_each(std::forward<TupleT>(tup), fun);
        }

        template <typename Fun, typename Tuple1, typename Tuple2>
        static inline
        void for_each2(Tuple1 &&t1, Tuple2 &&t2, Fun fun)
        {
            fun(std::get<std::tuple_size<typename remove_reference_and_cv<Tuple1>::type>::value - N>(std::forward<Tuple1>(t1)),
                std::get<std::tuple_size<typename remove_reference_and_cv<Tuple2>::type>::value - N>(std::forward<Tuple2>(t2)));
            tuple_<N-1>::for_each2(std::forward<Tuple1>(t1), std::forward<Tuple2>(t2), fun);
        }

        template <typename Tuple1, typename Tuple2, typename TupleT>
        static inline
        void zip(Tuple1 const &t1, Tuple2 const &t2, TupleT &t)
        {
            std::get<N-1>(t) = std::make_pair(std::get<N-1>(t1), std::get<N-1>(t2));
            tuple_<N-1>::zip(t1,t2,t);
        }

    };
    template <>
    struct tuple_<1>
    {
        template <typename Fun, typename TupleT>
        static inline
        void for_each(TupleT &&tup, Fun fun)
        {
            fun(std::get<std::tuple_size<
                            typename remove_reference_and_cv<TupleT>::type>::value - 1>(std::forward<TupleT>(tup)));
        }

        template <typename Fun, typename Tuple1, typename Tuple2>
        static inline
        void for_each2(Tuple1 &&t1, Tuple2 &&t2, Fun fun)
        {
            fun(std::get<std::tuple_size<typename remove_reference_and_cv<Tuple1>::type>::value - 1>(std::forward<Tuple1>(t1)),
                std::get<std::tuple_size<typename remove_reference_and_cv<Tuple2>::type>::value - 1>(std::forward<Tuple2>(t2)));
        }

        template <typename Tuple1, typename Tuple2, typename TupleT>
        static inline
        void zip(Tuple1 const &t1, Tuple2 const &t2, TupleT &t)
        {
            std::get<0>(t) = std::make_pair(std::get<0>(t1), std::get<0>(t2));
        }
    };

    template <typename Fun, typename TupleT>
    void tuple_for_each(TupleT &&tup, Fun fun)
    {
        tuple_<std::tuple_size<
                typename remove_reference_and_cv<TupleT>::type>::value>::for_each(std::forward<TupleT>(tup), fun);
    }

    /// tuple_for_each2

    template <typename Tp>
    constexpr Tp && min(Tp &&a, Tp &&b)
    {
        return a < b ? a : b;
    }

    template <typename Fun, typename Tuple1, typename Tuple2>
    void tuple_for_each2(Tuple1 &&t1, Tuple2 &&t2, Fun fun)
    {
        tuple_<min(std::tuple_size<typename remove_reference_and_cv<Tuple1>::type>::value,
                   std::tuple_size<typename remove_reference_and_cv<Tuple2>::type>::value)
              >::for_each2(std::forward<Tuple1>(t1), std::forward<Tuple2>(t2), fun);
    }

    /// tuple_call

    template <typename Fun, typename ...Ts>
    auto tuple_call_ret(Fun fun, std::tuple<Ts...>) -> decltype(fun(std::declval<Ts>()...));

    template <size_t N>
    struct tuple_args
    {
        template <typename Fun, typename TupleT, typename ...Ti>
        static inline
        auto call(Fun fun, TupleT &&tup, Ti && ... args)
        -> decltype(tuple_call_ret(fun,std::forward<TupleT>(tup)))
        {
            return tuple_args<N-1>::call(fun, std::forward<TupleT>(tup), std::get<N-1>(std::forward<TupleT>(tup)), std::forward<Ti>(args)...);
        }
    };
    template <>
    struct tuple_args<1>
    {
        template <typename Fun, typename TupleT, typename ...Ti>
        static inline
        auto call(Fun fun, TupleT &&tup, Ti && ... args)
        -> decltype(tuple_call_ret(fun,std::forward<TupleT>(tup)))
        {
            return fun(std::get<0>(std::forward<TupleT>(tup)), std::forward<Ti>(args)...);
        }
    };

    template <typename Fun, typename TupleT>
    auto tuple_call(Fun fun, TupleT &&tup)
    -> decltype(tuple_call_ret(fun, std::forward<TupleT>(tup)))
    {
        return tuple_args<std::tuple_size<
                typename remove_reference_and_cv<TupleT>::type>::value>::call(fun, std::forward<TupleT>(tup));
    }

    /// tuple zip!

    template <typename ...T1, typename ...T2>
    auto tuple_zip(std::tuple<T1...> const & t1, std::tuple<T2...> const &t2)
    -> std::tuple<std::pair<T1,T2>...>
    {
        std::tuple<std::pair<T1,T2>...> ret;
        tuple_<sizeof...(T1)>::zip(t1,t2,ret);
        return ret;
    }

} // namespace more


#endif /* _TUPLE_EXT_HPP_ */
