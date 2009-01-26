/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef TUPLE_HH
#define TUPLE_HH

#include <typelist.hh>

namespace mtp 
{
    // int2Type
    template <int n>
    struct int2Type 
    {
        enum { value = n };
    };

    // add_reference
    template <typename T>
    struct add_reference 
    {
        typedef T &type;
    };
    template <typename T>
    struct add_reference<T&>
    {
        typedef T &type; 
    };

    // my tuple! (inspired to that of Thomas Becker)
    template <typename L>
    class tuple 
    {
        typedef typename L::head          elem_type;
        typedef tuple<typename L::tail>   list_type;

        elem_type _M_elem;
        list_type _M_list;

    public:
    
        tuple() 
        : _M_elem(),
          _M_list()
        {}

        template <typename T1>
        tuple(const T1 &t1)
        : _M_elem(t1),
          _M_list()
        {}

        template <typename T1, typename T2>
        tuple(const T1 &t1, const T2 &t2)
        :  
          _M_elem(t1),
          _M_list(t2)
        {}

        template <typename T1, typename T2, typename T3>
        tuple(const T1 &t1, const T2 &t2, const T3 &t3)
        :  
          _M_elem(t1),
          _M_list(t2,t3)
        {}

        template <typename T1, typename T2, typename T3,
                  typename T4>
        tuple(const T1 &t1, const T2 &t2, const T3 &t3,
              const T4 &t4)
        :  
          _M_elem(t1),
          _M_list(t2,t3,t4)
        {}

        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5>
        tuple(const T1 &t1, const T2 &t2, const T3 &t3,
              const T4 &t4, const T5 &t5)
        :  
          _M_elem(t1),
          _M_list(t2,t3,t4,t5)
        {}

        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5, typename T6>
        tuple(const T1 &t1, const T2 &t2, const T3 &t3,
              const T4 &t4, const T5 &t5, const T6 &t6)
        :  
          _M_elem(t1),
          _M_list(t2,t3,t4,t5,t6)
        {}

        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5, typename T6,
                  typename T7>
        tuple(const T1 &t1, const T2 &t2, const T3 &t3,
              const T4 &t4, const T5 &t5, const T6 &t6,
              const T7 &t7)
        :  
          _M_elem(t1),
          _M_list(t2,t3,t4,t5,t6,t7)
        {}

        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5, typename T6,
                  typename T7, typename T8>
        tuple(const T1 &t1, const T2 &t2, const T3 &t3,
              const T4 &t4, const T5 &t5, const T6 &t6,
              const T7 &t7, const T8 &t8)
        :  
          _M_elem(t1),
          _M_list(t2,t3,t4,t5,t6,t7,t8)
        {}

        template <typename T1, typename T2, typename T3,
                  typename T4, typename T5, typename T6,
                  typename T7, typename T8, typename T9>
        tuple(const T1 &t1, const T2 &t2, const T3 &t3,
              const T4 &t4, const T5 &t5, const T6 &t6,
              const T7 &t7, const T8 &t8, const T9 &t9)
        :  
          _M_elem(t1),
          _M_list(t2,t3,t4,t5,t6,t7,t8,t9)
        {}

        template <int n>
        typename add_reference<typename TL::at<L,n>::type >::type
        get()
        { return __get(int2Type<n>()); }

        // __get is overloaded by means of int2Type
        template <int n>
        typename add_reference<typename TL::at<L,n>::type >::type
        __get(int2Type<n>)
        { return _M_list.__get(int2Type<n-1>()); }

        typename add_reference<elem_type>::type
        __get(int2Type<0>)
        { return _M_elem; }

        size_t size() const 
        { return TL::length<L>::value; }

    };

    template<>
    class tuple<TL::null> {};

    template <typename T1>
    static inline tuple<TYPELIST(T1)> 
    make_tuple (const T1& a1) 
    { 
        return tuple<TYPELIST(T1)> (a1); 
    }    
    
    template <typename T1, typename T2>
    static inline tuple<TYPELIST(T1,T2)> 
    make_tuple (const T1& a1, const T2& a2) 
    { 
        return tuple<TYPELIST(T1,T2)> (a1,a2); 
    }

    template <typename T1, typename T2, typename T3>
    static inline tuple<TYPELIST(T1,T2,T3)> 
    make_tuple (const T1& a1, const T2& a2, const T3& a3) 
    { 
        return tuple<TYPELIST(T1,T2,T3)> (a1,a2,a3); 
    }

    template <typename T1, typename T2, typename T3,
              typename T4>
    static inline tuple<TYPELIST(T1,T2,T3,T4)> 
    make_tuple (const T1& a1, const T2& a2, const T3& a3, 
                const T4& a4) 
    {
        return tuple<TYPELIST(T1,T2,T3,T4)> 
                (a1,a2,a3,a4);
    }    
    
    template <typename T1, typename T2, typename T3,
              typename T4, typename T5>
    static inline tuple<TYPELIST(T1,T2,T3,T4,T5)> 
    make_tuple (const T1& a1, const T2& a2, const T3& a3, 
                const T4& a4, const T5& a5) 
    {
        return tuple<TYPELIST(T1,T2,T3,T4,T5)> 
                (a1,a2,a3,a5,a5);
    }

    template <typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6>
    static inline tuple<TYPELIST(T1,T2,T3,T4,T5,T6)> 
    make_tuple (const T1& a1, const T2& a2, const T3& a3, 
                const T4& a4, const T5& a5, const T6& a6) 
    {
        return tuple<TYPELIST(T1,T2,T3,T4,T5,T6)> 
                (a1,a2,a3,a4,a5,a6); 
    }

    template <typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6,
              typename T7>
    static inline tuple<TYPELIST(T1,T2,T3,T4,T5,T6,T7)> 
    make_tuple (const T1& a1, const T2& a2, const T3& a3, 
                const T4& a4, const T5& a5, const T6& a6,
                const T7& a7) 
    {
        return tuple<TYPELIST(T1,T2,T3,T4,T5,T6,T7)> 
                (a1,a2,a3,a4,a5,a6,a7);
    }

    template <typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6,
              typename T7, typename T8>
    static inline tuple<TYPELIST(T1,T2,T3,T4,T5,T6,T7,T8)> 
    make_tuple (const T1& a1, const T2& a2, const T3& a3, 
                const T4& a4, const T5& a5, const T6& a6, 
                const T7& a7, const T8& a8) 
    {
        return tuple<TYPELIST(T1,T2,T3,T4,T5,T6,T7,T8)> 
                (a1,a2,a3,a4,a5,a6,a7,a8);
    }

    template <typename T1, typename T2, typename T3,
              typename T4, typename T5, typename T6,
              typename T7, typename T8, typename T9>
    static inline tuple<TYPELIST(T1,T2,T3,T4,T5,T6,T7,T8,T9)> 
    make_tuple (const T1& a1, const T2& a2, const T3& a3, 
                const T4& a4, const T5& a5, const T6& a6, 
                const T7& a7, const T8& a8, const T9& a9) 
    {
        return tuple<TYPELIST(T1,T2,T3,T4,T5,T6,T7,T8,T9)> 
                (a1,a2,a3,a4,a5,a6,a7,a8,a9);
    }
}

#endif /* TUPLE_HH */
