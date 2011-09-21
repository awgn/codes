 /* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef _MAYBE_HPP_
#define _MAYBE_HPP_ 

#include <stdexcept>
#include <type_traits>

namespace details 
{
    struct Nothing {};
    
    template <typename T1, typename T2>
    struct maybe_helper
    {
        static bool equal(const std::pair<T1, bool>& lhs, const std::pair<T2, bool>& rhs)
        { return (lhs.first == rhs.first && lhs.second && rhs.second) ||
            (!lhs.second && !rhs.second);
        }    
    };
    template <typename T1>
    struct maybe_helper<T1, Nothing>
    {
        static bool equal(const std::pair<T1,bool> &lhs, std::pair<Nothing,bool>) 
        { return !lhs.second; }
    };
    template <typename T2>
    struct maybe_helper<Nothing, T2>
    {
        static bool equal(std::pair<Nothing,bool>, const std::pair<T2,bool> &rhs)
        { return !rhs.second; }
    };
    template <>
    struct maybe_helper<Nothing, Nothing>
    {
        static bool equal(const std::pair<Nothing,bool> &, const std::pair<Nothing,bool> &)
        { return true; }
    };
}


template <typename Tp>
class Maybe {

public:

    Maybe()
    : m_value(), m_state(false)
    {}

    Maybe(const Maybe<details::Nothing>&)
    : m_value(), m_state(false)
    {}

    explicit Maybe(Tp value)
    : m_value(std::move(value)), m_state(true)
    {}

    Maybe& operator=(const Maybe<details::Nothing>&)
    {
        m_state = false;
        return *this;
    }    

    template <typename T>
    bool operator==(const Maybe<T> &other)
    {
        return details::maybe_helper<Tp,T>::equal(std::make_pair(value(), state()), 
                                                  std::make_pair(other.value(), other.state())); 
    }
    template <typename T>
    bool operator!=(const Maybe<T> &other)
    {
        return !(*this == other);
    }

    operator Tp()
    {
        if (!m_state)
            throw std::runtime_error("Maybe<>: Nothing");
        return m_value;
    }

    explicit operator bool() = delete;

    Tp value() const
    { 
        return m_value;
    }

    bool state() const
    {
        return m_state;
    }

private:
    Tp   m_value;
    bool m_state;
};


template <typename Tp>
inline Maybe<Tp> Just(Tp && value)
{
    return Maybe<Tp>(std::forward<Tp>(value));
}

namespace 
{
    Maybe<details::Nothing> Nothing = Maybe<details::Nothing>();
}


#endif /* _MAYBE_HPP_ */
