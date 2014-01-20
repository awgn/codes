#pragma once

#include <limits>
#include <stdexcept>
#include <type_traits>


inline namespace more_inline
{
    // std::numeric_limits<>::max() and min() for enum types is defined as 0.
    // The following limits<> implementation provides the numeric_limits for
    // numeric types and the limits of the underlying types for enum.
    //
    // Nicola

    template <typename Tp, class = void> struct core_type;
    template <typename Tp>
    struct core_type<Tp, typename std::enable_if<!std::is_enum<Tp>::value>::type>
    {
        typedef Tp type;
    };

    template <typename Tp>
    struct core_type<Tp, typename std::enable_if<std::is_enum<Tp>::value>::type>
    {
#if (__GNUC_MINOR_ > 6) || defined(__clang__)
        typedef typename std::underlying_type<Tp>::type type;
#else
        typedef unsigned int type;
#endif
    };

    template <typename Tp>
    struct limits
    {
        static typename core_type<Tp>::type
        max()
        {
            return std::numeric_limits<typename core_type<Tp>::type>::max();
        }

        static typename core_type<Tp>::type
        min()
        {
            return std::numeric_limits<typename core_type<Tp>::type>::min();
        }
    };

    // R and T have the same type
    //

    template <typename R, typename T>
    typename std::enable_if<
        std::is_same<R,T>::value,
    R>::type
    safe_cast(T value)
    {
        return value;
    }

    // R and T: both signed
    //

    template <typename R, typename T>
    typename std::enable_if<
        !std::is_same<R,T>::value &&
         std::is_signed<R>::value &&
         std::is_signed<T>::value,
    R>::type
    safe_cast(T value)
    {
        if ((value > limits<R>::max()) ||
            (value < limits<R>::min()) )
            throw std::runtime_error("safe_cast: unsafe conversion");

        return static_cast<R>(value);
    }

    // R and T: both unsigned
    //

    template <typename R, typename T>
    typename std::enable_if<
        !std::is_same<R,T>::value &&
        !std::is_signed<R>::value &&
        !std::is_signed<T>::value,
    R>::type
    safe_cast(T value)
    {
        if ( value > limits<R>::max() )
            throw std::runtime_error("safe_cast: unsafe conversion");

        return static_cast<R>(value);
    }

    // R and T: differ in signedness, but safe (no weird conversions)
    //

    template <typename R, typename T>
    typename std::enable_if<
        !std::is_same<R,T>::value &&
        !std::is_signed<R>::value &&
         std::is_signed<T>::value &&
         (sizeof(R) < sizeof(T)),
    R>::type
    safe_cast(T value)
    {
        if (value < 0 || value > static_cast<T>(limits<R>::max()))
            throw std::runtime_error("safe_cast: unsafe conversion");

        return static_cast<R>(value);
    }


    template <typename R, typename T>
    typename std::enable_if<
        !std::is_same<R,T>::value &&
         std::is_signed<R>::value &&
        !std::is_signed<T>::value &&
        (sizeof(R) > sizeof(T)),
    R>::type
    safe_cast(T value)
    {
        return static_cast<R>(value);
    }

    // R and T: differ in signedness, with unsafe but controlled conversion
    //

    template <typename R, typename T>
    typename std::enable_if<
        !std::is_same<R,T>::value &&
        !std::is_signed<R>::value &&
         std::is_signed<T>::value &&
        (sizeof(R) >= sizeof(T)),
    R>::type
    safe_cast(T value)
    {
        if (value < 0)
            throw std::runtime_error("safe_cast: unsafe conversion");

        return static_cast<R>(value);
    }

    template <typename R, typename T>
    typename std::enable_if<
        !std::is_same<R,T>::value &&
         std::is_signed<R>::value &&
        !std::is_signed<T>::value &&
        (sizeof(R) <= sizeof(T)),
    R>::type
    safe_cast(T value)
    {
        if (value > static_cast<T>(limits<R>::max()))
            throw std::runtime_error("safe_cast: unsafe conversion");

        return static_cast<R>(value);
    }
} // namespace more_inline


