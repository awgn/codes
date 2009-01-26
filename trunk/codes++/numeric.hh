/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef NUMERIC_HH
#define NUMERIC_HH

namespace mtp 
{
    template <typename T, T a, T b>
    struct max 
    { 
        static const T value = ( a > b ? a : b );
    };

    template <typename T, T a, T b>
    struct min 
    {
        static const T value = ( a < b ? a : b );
    };

    // factorial
    template <int N>
    struct factorial 
    {
        enum { value = N * factorial<N - 1>::value };
    };
    template <>
    struct factorial<0> 
    {
        enum { value = 1 };
    };

    // comb(n,m) = m!/( n! * (m-n)!)
    template <int N, int M>
    struct comb 
    {
        enum { value = factorial<M>::value/(factorial<N>::value * factorial<M-N>::value) };
    };

    // sqrt<n>::value >= sqrt(n)
    template <int N, int I=1>
    struct sqrt 
    {
        enum { value = (I*I<N) ? sqrt<N,I+1>::value : I };
    };
    template <int N>
    struct sqrt<N,N> 
    {
        enum { value = N};
    };

    // pow2
    template <int N>
    struct pow2 
    {
        enum { value = 1 << N };
    };

    // log2
    template <int n>
    struct log2
    {
        enum { value = 1 + log2< n >> 1 >::value };
    };
    template <>
    struct log2<1>
    {
        enum { value = 0 };
    };

    // log10
    template <int n>
    struct log10
    {
        enum { value = (n >= 1000000000) ? 9 : (n >= 100000000) ? 8 : (n >= 10000000) ? 7 :
            (n >= 1000000) ? 6 : (n >= 100000) ? 5 : (n >= 10000) ? 4 :
            (n >= 1000) ? 3 : (n >= 100) ? 2 : (n >= 10) ? 1 : 0 };
    };

    // is_pow2
    template <int N>
    struct is_pow2
    {
        enum { value = ((N & (N - 1)) == 0) };
    };
    template<>
    struct is_pow2<0>
    {
        enum { value = 0 };
    };

    // ceil2 ( i <= 2^x )
    template <int i>
    struct ceil2_r 
    {
        enum { value = ceil2_r< (i>>1) >::value << 1 };
    };
    template <>
    struct ceil2_r<0> 
    {
        enum { value = 1 };
    };
    template <int i>
    struct ceil2 
    {
        enum { value = ceil2_r< (i-1>>1) >::value << 1 };
    };
    template <>
    struct ceil2<0> 
    {
        enum { value = 0 };
    };

    // floor2 ( 2^x <= i )
    template <int i>
    struct floor2_r 
    {
        enum { value = floor2_r< (i>>1) >::value << 1 };
    };
    template <>
    struct floor2_r<0> 
    {
        enum { value = 1 };
    };
    template <int i>
    struct floor2 
    {
        enum { value = floor2_r< (i>>1) >::value };
    };
    template <>
    struct floor2<0> 
    {
        enum { value = 0 };
    };

    // is_prime 
    template <int p, int i>
    struct is_prime 
    {
        enum { value = (p==2) || (p%i) && is_prime<(i>2?p:0),i-1>::value }; 
    }; 
    template<>
    struct is_prime<0,0> 
    {
        enum { value = 1 };
    }; 
    template<>
    struct is_prime<0,1> 
    {
        enum { value = 1 };
    }; 

    // binary
    template <unsigned long N> 
    struct binary
    {
        enum { value = binary<N/10>::value * 2 + N%10 };
    }; 
    template <>                           
    struct binary<0>                      
    {
        enum { value = 0 };
    };

    // fibonacci
    template<int I>
    struct fibonacci
    {
        enum { value = fibonacci<I-1>::value + fibonacci<I-2>::value };
    };
    template<>
    struct fibonacci<1>
    {
        enum { value = 1 };
    };
    template<>
    struct fibonacci<0>
    {
        enum { value = 1 };
    };

} // namespace numeric

#endif /* NUMERIC_HH */
