/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _SIGNAL_SLOT_HPP_
#define _SIGNAL_SLOT_HPP_ 

#include <functional>
#include <algorithm>
#include <vector>

using namespace std::placeholders;

namespace more { 

    template <int N> struct bind_all;
    template <>
    struct bind_all<0>
    {
        template <typename T, typename Fn>
        static auto
        to(Fn c, T  * that) -> decltype(std::bind(c, that))
        {
           return std::bind(c, that); 
        }
    };
    template <>
    struct bind_all<1>
    {
        template <typename T, typename Fn>
        static auto
        to(Fn c, T  * that) -> decltype(std::bind(c, that, _1))
        {
           return std::bind(c, that, _1); 
        }
    };
    template <>
    struct bind_all<2>
    {
        template <typename T, typename Fn>
        static auto
        to(Fn c, T  * that) -> decltype(std::bind(c, that, _1, _2))
        {
           return std::bind(c, that, _1, _2); 
        }
    };
    template <>
    struct bind_all<3>
    {
        template <typename T, typename Fn>
        static auto
        to(Fn c, T  * that) -> decltype(std::bind(c, that, _1, _2, _3))
        {
           return std::bind(c, that, _1, _2, _3); 
        }
    };
    template <>
    struct bind_all<4>
    {
        template <typename T, typename Fn>
        static auto
        to(Fn c, T  * that) -> decltype(std::bind(c, that, _1, _2, _3, _4))
        {
           return std::bind(c, that, _1, _2, _3, _4); 
        }
    };
    template <>
    struct bind_all<5>
    {
        template <typename T, typename Fn>
        static auto
        to(Fn c, T  * that) -> decltype(std::bind(c, that, _1, _2, _3, _4, _5))
        {
           return std::bind(c, that, _1, _2, _3, _4, _5); 
        }
    };

    template <typename R, typename ...Ti> class slot;
    template <typename R, typename ...Ti>
    class slot<R(Ti...)>
    {
    public:
        template <typename C>
        slot(C *that, R(C::*c)(Ti...))
        : m_fun(bind_all<sizeof...(Ti)>::to(c, that))
        {}

        R
        operator()(Ti && ...args)
        {
            return m_fun(std::forward<Ti>(args)...);
        }

    private:
        std::function<R(Ti...)> m_fun;
    };


    template <typename R, typename ...Ti> class signal;
    template <typename R, typename ...Ti>
    class signal<R(Ti...)>
    {
    public:
        signal()
        {}

        void connect(slot<R(Ti...)> &_slot)
        {
            m_slots.push_back(&_slot);
        }
        
        void disconnect(slot<R(Ti...)> &_slot)
        {
            m_slots.erase(std::remove(m_slots.begin(), m_slots.end(), &_slot), m_slots.end());
        }

        std::vector<R>
        operator()(Ti && ...args)
        {
            std::vector<R> ret;
            for(auto it = m_slots.begin(), it_e = m_slots.end(); it != it_e; ++it)
            {
                ret.push_back((*it)->operator()(std::forward<Ti>(args)...));
            }
            return ret;
        }

    private:
        std::vector<slot<R(Ti...)> *> m_slots;
    };

} // namespace more


#endif /* _SIGNAL_SLOT_HPP_ */
