/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#pragma once 

#include <string>
#include <sstream>
#include <stdexcept>
#include <memory>
#include <cassert>
#include <array>
#include <chrono>

#include <iostream>

#include <type_traits.hpp>
#include <cxxabi.h>

inline namespace more_read {

    //////////////////////////////////////////////////////
    //
    // forward declaration...
    //

    template <typename T, typename CharT, typename Traits>
    T read(std::basic_istream<CharT,Traits>&in);

    template <typename T>
    std::pair<T, std::string> read(std::string const &ref);

    namespace details
    {
        template <typename Tp>
        struct decay
        {
            typedef typename std::decay<Tp>::type type;
        };
        template <typename T1, typename T2>
        struct decay<std::pair<T1,T2>>
        {
            typedef std::pair<typename std::decay<T1>::type, 
                              typename std::decay<T2>::type> type;
        };
        template <typename ...Ts>
        struct decay<std::tuple<Ts...>>
        {
            typedef std::tuple<typename std::decay<Ts>::type...> type;
        };

        inline std::string
        demangle(const char * name)
        {
            int status;
            std::unique_ptr<char, void(*)(void *)> ret(abi::__cxa_demangle(name,0,0, &status), ::free);
            if (status < 0) {
                return std::string(1,'?');
            }
            return std::string(ret.get());
        }

        template <typename Tp>
        std::string
        error(const char *msg)
        {
            return "read<" + demangle(typeid(Tp).name()) + ">: " + msg;
        }

        template <typename T, int N>
        struct read_on
        {
            template <typename CharT, typename Traits>
            static inline
            void apply(T &tupl, std::basic_istream<CharT, Traits> &in)
            {
                constexpr size_t I = std::tuple_size<T>::value - N;

                std::get<I>(tupl) = read<typename std::tuple_element<I, T>::type>(in);

                read_on<T,N-1>::apply(tupl, in);
            }
        }; 
        template <typename T>
        struct read_on<T, 0>
        {
            template <typename CharT, typename Traits>
            static inline
            void apply(T &, std::basic_istream<CharT, Traits> &)
            {}
        };
        
        // enabled for std::vector, std::dequeue, std::list
        // 
        template <typename C, typename V>
        typename std::enable_if<!more::traits::has_key_type<C>::value && 
                                !more::traits::has_container_type<C>::value, bool>::type 
        insert(C &cont, V &&value)
        {
            cont.push_back(std::forward<V>(value));
            return true;
        }

        // enabled for std::stack, std::queue, std::priority_queue
        // 
        template <typename C, typename V>
        typename std::enable_if<!more::traits::has_key_type<C>::value && 
                                more::traits::has_container_type<C>::value, bool>::type 
        insert(C &cont, V &&value)
        {
            cont.push(std::forward<V>(value));
            return true;
        }

        // enabled for std::set, std::multiset, std::unordered_set,
        // std::unordered_multiset
        // 
        template <typename C, typename V>
        typename std::enable_if<more::traits::has_key_type<C>::value, bool>::type 
        insert(C &cont, V && value)
        {
            return cont.insert(std::forward<V>(value)).second;
        }

        // enabled for std::map, std::multimap, std::unordered_map,
        // std::unordered_multimap
        // 
        template <typename C, typename T, typename V>
        typename std::enable_if<more::traits::has_key_type<C>::value, bool>::type 
        insert(C &cont, std::pair<T,V> && value)
        {
            return cont.insert(std::move(value)).second;
        }

    } // namespace details


    // utility for tag dispatching...
    //
    
    template <typename Tp>
    struct tag
    {
        typedef Tp type;
    };

    
    // consume a specified char, return true or false 
    //
   
    template <typename CharT, typename Traits>
    bool consume(char c, std::basic_istream<CharT,Traits>& in)
    {
        decltype(c) _c;

        if(!(in >> std::ws)) 
            return false;

        _c = in.peek();
        if (!in) 
            return false;

        if (c == _c) {
            in.get();
            assert(in);
            return true;
        }

        return false;
    }
    
    // consume a specified string, throw an exception if not matching 
    //
    
    template <typename CharT, typename Traits>
    void consume(const char *s, std::basic_istream<CharT, Traits> &in)
    {
        std::string _s; 
        if (!(in >> _s) || _s.compare(s) != 0) 
            throw std::runtime_error("read: " + _s + " (" + s  + ") parse error");
    }                                                            

    //
    // specializations for specific types...
    //

    template <typename CharT, typename Traits>
    bool 
    read(tag<bool>, std::basic_istream<CharT,Traits>&in)
    {
        bool ret;
        in >> std::noboolalpha;

        if (!(in >> ret)) {
            in.clear();
            if (!(in >> std::boolalpha >> ret))
                throw std::runtime_error(details::error<bool>("parse error"));
        }
        
        return ret;
    }

    // pair<T1,T2>:
    //
    
    template <typename T1, typename T2, typename CharT, typename Traits>
    std::pair<T1,T2> 
    read(tag<std::pair<T1,T2>>, std::basic_istream<CharT,Traits>&in)
    {
        if (consume('(', in)) {

            T1 a = read<T1>(in); 
            T2 b = read<T2>(in);

            if (!consume(')', in))
                throw std::runtime_error(details::error<std::pair<T1,T2>>("parse error"));

            return std::make_pair(std::move(a), std::move(b));
        }
        else if (consume('[', in)) {

            T1 a = read<T1>(in); consume("->", in); 
            T2 b = read<T2>(in);

            if (!consume(']', in))
                throw std::runtime_error(details::error<std::pair<T1,T2>>("parse error"));

            return std::make_pair(std::move(a), std::move(b));
        }
        else {

            T1 a = read<T1>(in); consume("->", in); 
            T2 b = read<T2>(in);

            if (!consume(']', in))
                throw std::runtime_error(details::error<std::pair<T1,T2>>("parse error"));

            return std::make_pair(std::move(a), std::move(b));
        }
    }
    
    // std::array<T,N>:
    //

    template <typename T, size_t N, typename CharT, typename Traits>
    std::array<T,N> 
    read(tag<std::array<T,N>>, std::basic_istream<CharT,Traits>&in)
    {
        std::array<T, N> ret;

        if (!consume('[', in))
            throw std::runtime_error(details::error<std::array<T,3>>("parse error"));

        for(auto & e : ret)
        {
            e = read<T>(in);
        }

        if (!consume(']', in))
            throw std::runtime_error(details::error<std::array<T,3>>("parse error"));

        return ret;
    }

    // std::chrono::duration<Rep, Period>:
    //
    
    template <typename Rep, typename Period, typename CharT, typename Traits>
    std::chrono::duration<Rep, Period>
    read(tag<std::chrono::duration<Rep, Period>>, std::basic_istream<CharT,Traits>&in)
    {
        typedef std::chrono::duration<Rep, Period> Duration;
        
        int64_t value; char c; std::string unit;

        if (!(in >> value >> c >> unit))
            throw std::runtime_error(details::error<std::chrono::duration<Rep, Period>>("parse error"));

        if (unit.compare("ns") == 0)
            return std::chrono::duration_cast<Duration>(std::chrono::nanoseconds(value));

        if (unit.compare("us") == 0)
            return std::chrono::microseconds(value);
        
        if (unit.compare("ms") == 0)
            return std::chrono::milliseconds(value);

        if (unit.compare("s") == 0)
            return std::chrono::seconds(value);
        
        if (unit.compare("m") == 0)
            return std::chrono::minutes(value);

        if (unit.compare("h") == 0)
            return std::chrono::hours(value);

        throw std::runtime_error(details::error<std::chrono::duration<Rep, Period>>("parse error"));
    }
             
    // std::chrono::time_point<Clock, Duration>:
    //
    
    template <typename Clock, typename Dur, typename CharT, typename Traits>
    std::chrono::time_point<Clock, Dur>
    read(tag<std::chrono::time_point<Clock, Dur>>, std::basic_istream<CharT,Traits>&in)
    {
        return std::chrono::time_point<Clock, Dur>( read<Dur>(in) );
    }
    
    // std::tuple<Ts...>:
    //

    template <typename ...Ts, typename CharT, typename Traits>
    std::tuple<Ts...> 
    read(tag<std::tuple<Ts...>>, std::basic_istream<CharT,Traits>&in)
    {
        std::tuple<Ts...> ret;

        if (!consume('(', in))
            throw std::runtime_error(details::error<std::tuple<Ts...>>("parse error"));

        details::read_on<std::tuple<Ts...>, sizeof...(Ts)>::apply(ret, in);

        if (!consume(')', in))
            throw std::runtime_error(details::error<std::tuple<Ts...>>("parse error"));

        return ret;
    }
 
    // std::string:
    //

    template <typename CharT, typename Traits>
    std::string 
    read(tag<std::string>, std::basic_istream<CharT,Traits>&in)
    {
        typedef std::string::traits_type traits_type;

        in >> std::noskipws >> std::ws;

        std::string str; str.reserve(32);
        traits_type::char_type c;

        enum class pstate { null, raw_string, escaped_char, quoted_string, escaped_char2 };
        auto state = pstate::null;

        auto raw_char = [](traits_type::char_type c) -> bool 
        {
            return std::isalnum(c) || traits_type::eq(c, '_') || traits_type::eq(c, '-');
        };

        bool stop = false, quoted = false; 

        while (!stop)
        {
            c = static_cast<traits_type::char_type>(in.peek());
            if (c == traits_type::eof())
                break;

            switch(state)
            {
            case pstate::null:
                {
                    if (c == '"')       { in.get(); state = pstate::quoted_string; break;}
                    if (raw_char(c))    { in.get(); state = pstate::raw_string; str.push_back(c); break;}
                    if (c == '\\')      { in.get(); state = pstate::escaped_char;  break;} 

                    stop = true;

                } break;

            case pstate::raw_string:
                {
                    if (raw_char(c))    { in.get(); state = pstate::raw_string; str.push_back(c); break; }
                    if (c == '\\')      { in.get(); state = pstate::escaped_char; break; } 

                    stop = true;

                } break;

            case pstate::escaped_char:
                {
                    in.get(); state = pstate::raw_string; str.push_back(c);

                } break;

            case pstate::quoted_string:
                {
                    if (c == '"')       { in.get(); state = pstate::quoted_string; stop = true; quoted = true; break;}
                    if (c == '\\')      { in.get(); state = pstate::escaped_char2; break;} 

                    in.get(); str.push_back(c); 

                } break;

            case pstate::escaped_char2:
                {
                    in.get(); str.push_back(c); state = pstate::quoted_string;

                } break;
            }
        }

        in >> std::skipws;

        if (!quoted && str.empty())
            throw std::runtime_error(details::error<std::string>("parse error"));

        return str;
    }
    
    // container:
    //
    
    template <typename Tp, typename CharT, typename Traits>
    typename std::enable_if<
        more::traits::is_container<Tp>::value,
    Tp>::type
    read(tag<Tp>, std::basic_istream<CharT,Traits>& in)
    {
        Tp ret;

        if (!consume('[', in))
            throw std::runtime_error(details::error<Tp>("parse error"));

        while (!consume(']', in))
        {
            details::insert(ret, read<typename Tp::value_type>(in));
        }

        return ret;
    }
    
    // generic types supporting extraction operator>>():
    //
    
    template <typename Tp, typename CharT, typename Traits>
    typename std::enable_if<
        !more::traits::is_container<Tp>::value,
    Tp>::type
    read(tag<Tp>, std::basic_istream<CharT,Traits>& in)
    {
        Tp ret{};
        if (!(in >> ret))
            throw std::runtime_error(details::error<int>("parse error"));
        return ret;
    }
    
    //////////////////////////////////////////////////////////////////////////////////
    //
    // interfaces...
    //
    
    template <typename T, typename CharT, typename Traits>
    T read(std::basic_istream<CharT,Traits>&in)
    {
        return read(tag<typename details::decay<T>::type>(), in);
    }


    template <typename T>
    std::pair<T, std::string> read(std::string const &ref)
    {
        std::istringstream in(ref);

        auto value = read<T>(in);
        auto res   = in.str();
        auto count = in.rdbuf()->in_avail();
        
        return std::make_pair(std::move(value), res.substr(res.size()-count, count));
    }

} // namespace more_read
