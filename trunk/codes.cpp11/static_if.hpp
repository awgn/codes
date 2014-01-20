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

#include <utility>

template <bool V>
struct static_if
{
    template <typename F, typename ...Ts>
    static_if<V> &
    then_(Ts&&... args)
    {
        return F()(std::forward<Ts>(args)...), *this;
    }

    template <typename F, typename ...Ts>
    static_if<V> &
    else_(Ts&&...)
    {
        return *this;
    }
};


template <>
struct static_if<false>
{
    template <typename F, typename ...Ts>
    static_if<false> &
    then_(Ts&&...)
    {
        return *this;
    }

    template <typename F, typename ...Ts>
    static_if<false> &
    else_(Ts&&... args)
    {
        return F()(std::forward<Ts>(args)...), *this;
    }
};
