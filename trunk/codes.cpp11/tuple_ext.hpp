#ifndef _TUPLE_EXT_HPP_
#define _TUPLE_EXT_HPP_ 

#include <cstddef>
#include <tuple>

namespace more { 

    // can't use simple recursion with variadic template functions
    // because there's no way to get the tail of a tuple!! Nicola 
    //
    
    template <size_t N>
    struct tuple_
    {
        template <typename F, typename ...Ts>
        static inline 
        void for_each(std::tuple<Ts...> &t, F f) 
        {
            f(std::get<sizeof...(Ts) - N>(t));
            tuple_<N-1>::for_each(t, f);
        }
        template <typename F, typename ...Ts>
        static inline 
        void for_each(std::tuple<Ts...> const &t, F f) 
        {
            f(std::get<sizeof...(Ts) - N>(t));
            tuple_<N-1>::for_each(t, f);
        }
    };
    template <>
    struct tuple_<1>
    {
        template <typename F, typename ...Ts>
        static inline 
        void for_each(std::tuple<Ts...> &t, F f)
        { 
            f(std::get<sizeof...(Ts) - 1>(t));
        }
        template <typename F, typename ...Ts>
        static inline 
        void for_each(std::tuple<Ts...> const &t, F f)
        { 
            f(std::get<sizeof...(Ts) - 1>(t));
        }
    };

    template <typename F, typename ...Ts>
    void tuple_for_each(std::tuple<Ts...> &t, F f)
    {
        tuple_<sizeof...(Ts)>::for_each(t, f);
    }   
    template <typename F, typename ...Ts>
    void tuple_for_each(std::tuple<Ts...> const &t, F f)
    {
        tuple_<sizeof...(Ts)>::for_each(t, f);
    }   

    
    template <size_t N>
    struct tuple_args 
    {
        template <typename Fun, typename ...Ts, typename ...Ti>
        static 
        auto call(Fun fun, std::tuple<Ts...> &tup, Ti && ... args)
        -> decltype(fun(std::declval<Ts>()...))
        {
            return tuple_args<N-1>::call(fun, tup, std::get<N-1>(tup), std::forward<Ti>(args)...);
        }
    };
    template <>
    struct tuple_args<1> 
    {
        template <typename Fun, typename ...Ts, typename ...Ti>
        static 
        auto call(Fun fun, std::tuple<Ts...> &tup, Ti && ... args) 
        -> decltype(fun(std::declval<Ts>()...))
        {
            return fun(std::get<0>(tup), std::forward<Ti>(args)...);
        }
    };

    template <typename Fun, typename ...Ts>
    auto tuple_call(Fun fun, std::tuple<Ts...> & tup)
    -> decltype(fun(std::declval<Ts>()...))
    {
        return tuple_args<sizeof...(Ts)>::call(fun, tup); 
    }

} // namespace more


#endif /* _TUPLE_EXT_HPP_ */
