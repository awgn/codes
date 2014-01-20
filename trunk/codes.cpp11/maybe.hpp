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

#include <cxxabi.hpp>  // more!

#include <stdexcept>
#include <type_traits>
#include <memory>

#include <iostream>


namespace details
{
    struct Nothing {};

    template <typename T1, typename T2>
    struct maybe_helper
    {
        static bool equal(const std::pair<T1, bool>& lhs, const std::pair<T2, bool>& rhs)
        {
            return (lhs.first == rhs.first && lhs.second && rhs.second) ||
                   (!lhs.second && !rhs.second);
        }
        static bool less(const std::pair<T1, bool>& lhs, const std::pair<T2, bool>& rhs)
        {
           return rhs.second && (!lhs.second || lhs.first < rhs.first);
        }
    };
    template <typename T1>
    struct maybe_helper<T1, Nothing>
    {
        static bool equal(const std::pair<T1,bool> &lhs, std::pair<Nothing,bool>)
        {
            return !lhs.second;
        }

        static bool less(const std::pair<T1,bool>, std::pair<Nothing,bool>)
        {
            return false;
        }
    };
    template <typename T2>
    struct maybe_helper<Nothing, T2>
    {
        static bool equal(std::pair<Nothing,bool>, const std::pair<T2,bool> &rhs)
        {
            return !rhs.second;
        }

        static bool less(std::pair<Nothing,bool>, const std::pair<T2,bool> &rhs)
        {
            return rhs.second;
        }
    };
    template <>
    struct maybe_helper<Nothing, Nothing>
    {
        static bool equal(const std::pair<Nothing,bool>, const std::pair<Nothing,bool>)
        {
            return true;
        }

        static bool less(const std::pair<Nothing,bool>, const std::pair<Nothing,bool>)
        {
            return false;
        }
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
    bool operator==(const Maybe<T> &other) const
    {
        return details::maybe_helper<Tp,T>::equal(std::make_pair(unsafeJust(), isJust()),
                                                  std::make_pair(other.unsafeJust(), other.isJust()));
    }
    template <typename T>
    bool operator!=(const Maybe<T> &other) const
    {
        return !(*this == other);
    }

    template <typename T>
    bool operator<(const Maybe<T> &other) const
    {
        return details::maybe_helper<Tp,T>::less(std::make_pair(unsafeJust(), isJust()),
                                                  std::make_pair(other.unsafeJust(), other.isJust()));
    }
    template <typename T>
    bool operator<=(const Maybe<T> &other) const
    {
        return !(other < *this);
    }
    template <typename T>
    bool operator>=(const Maybe<T> &other) const
    {
        return !(*this < other);
    }
    template <typename T>
    bool operator>(const Maybe<T> &other) const
    {
        return other < *this;
    }

    explicit operator bool() = delete;

    explicit operator Tp()
    {
        return fromJust();
    }

    bool isJust() const
    {
        return m_state == true;
    }

    bool isNothing() const
    {
        return m_state == false;
    }

    Tp fromJust() const
    {
        if (!m_state)
            throw std::runtime_error("Maybe<" + demangle(typeid(Tp).name()) + ">: Nothing");
        return m_value;
    }

    Tp unsafeJust() const
    {
        return m_state ? m_value : Tp();
    }

    template <typename Fn>
    auto fmap(Fn fun) -> decltype(Maybe<decltype(fun(std::declval<Tp>()))>(fun(this->fromJust())));

private:
    Tp   m_value;
    bool m_state;
};


namespace
{
    Maybe<details::Nothing> Nothing = Maybe<details::Nothing>();
}


template <typename Fn,  typename Tp>
inline
Maybe<Tp> maybe(Tp b, Fn f, Maybe<Tp> const &a)
{
    if (a == Nothing)
        return b;
    else
        return Just(f(a.fromJust()));
}


template <typename Tp>
inline Maybe<typename std::remove_reference<Tp>::type> Just(Tp && value)
{
    return Maybe<typename std::remove_reference<Tp>::type
                    >(std::forward<Tp>(value));
}

template <typename Tp>
template <typename Fn>
auto
Maybe<Tp>::fmap(Fn fun) -> decltype(Maybe<decltype(fun(std::declval<Tp>()))>(fun(this->fromJust())))
{
    if (isNothing())
        return Nothing;
    else
        return Just<decltype(fun(std::declval<Tp>()))>(fun(fromJust()));
}

template <typename CharT, typename Traits>
typename std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT,Traits> &out, const Maybe<details::Nothing>&)
{
     return out << "Nothing";
}
template <typename CharT, typename Traits, typename T>
typename std::basic_ostream<CharT, Traits> &
operator<<(std::basic_ostream<CharT,Traits> &out, const Maybe<T>& other)
{
      return (other.isNothing()) ?  (out << "Nothing") : (out << "Just " << other.fromJust());
}

template <typename CharT, typename Traits, typename T>
typename std::basic_istream<CharT, Traits> &
operator>>(std::basic_istream<CharT,Traits> &in, Maybe<T>& other)
{
    std::string word;
    auto pos = in.tellg();
    if (in >> word)
    {
        if (!word.compare("Nothing")) {
            other = Nothing;
            return in;
        }
        if (!word.compare("Just")) {
            T value;
            if (in >> value)
            {
                other = Just(value);
                return in;
            }
        }
        if(!in.seekg(pos))
            throw std::runtime_error("Maybe: seekg");
        in.setstate(std::ios_base::failbit);
    }
    return in;
}

#endif /* _MAYBE_HPP_ */
