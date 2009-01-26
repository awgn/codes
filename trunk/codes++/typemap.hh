/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef _TYPEMAP_HH_
#define _TYPEMAP_HH_ 

#include <iostream>       
#include <string>

/* the so-called __VA_NARG__ (PP_NARG) macro from the thread at 
   http://groups.google.com/group/comp.std.c/browse_frm/thread/77ee8c8f92e4a3fb 
 */

#ifndef PP_NARG
#define PP_NARG(...) \
         PP_NARG_(__VA_ARGS__,PP_RSEQ_N())
#define PP_NARG_(...) \
         PP_ARG_N(__VA_ARGS__)
#define PP_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define PP_RSEQ_N() \
         63,62,61,60,                   \
         59,58,57,56,55,54,53,52,51,50, \
         49,48,47,46,45,44,43,42,41,40, \
         39,38,37,36,35,34,33,32,31,30, \
         29,28,27,26,25,24,23,22,21,20, \
         19,18,17,16,15,14,13,12,11,10, \
         9,8,7,6,5,4,3,2,1,0 
#endif /* PP_NARG */

#ifndef PASTE
#define PASTE(a,b)  a ## b
#define XPASTE(a,b) PASTE(a,b)
#endif /* PASTE */

// typelist macro helper ala loki...
//

#ifndef TYPEMAP
#define TYPEMAP_2(a,b,...)            mtp::TM::typemap<a,b,mtp::TM::null>
#define TYPEMAP_4(a,b,...)            mtp::TM::typemap<a,b,TYPEMAP_2(__VA_ARGS__) >
#define TYPEMAP_6(a,b,...)            mtp::TM::typemap<a,b,TYPEMAP_4(__VA_ARGS__) >
#define TYPEMAP_8(a,b,...)            mtp::TM::typemap<a,b,TYPEMAP_6(__VA_ARGS__) >
#define TYPEMAP_10(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_8(__VA_ARGS__) >
#define TYPEMAP_12(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_10(__VA_ARGS__) >
#define TYPEMAP_14(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_12(__VA_ARGS__) >
#define TYPEMAP_16(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_14(__VA_ARGS__) >
#define TYPEMAP_18(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_16(__VA_ARGS__) >
#define TYPEMAP_20(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_18(__VA_ARGS__) >
#define TYPEMAP_22(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_20(__VA_ARGS__) >
#define TYPEMAP_24(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_22(__VA_ARGS__) >
#define TYPEMAP_26(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_24(__VA_ARGS__) >
#define TYPEMAP_28(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_26(__VA_ARGS__) >
#define TYPEMAP_30(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_28(__VA_ARGS__) >
#define TYPEMAP_32(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_30(__VA_ARGS__) >
#define TYPEMAP_34(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_32(__VA_ARGS__) >
#define TYPEMAP_36(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_34(__VA_ARGS__) >
#define TYPEMAP_38(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_36(__VA_ARGS__) >
#define TYPEMAP_40(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_38(__VA_ARGS__) >
#define TYPEMAP_42(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_40(__VA_ARGS__) >
#define TYPEMAP_44(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_42(__VA_ARGS__) >
#define TYPEMAP_46(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_44(__VA_ARGS__) >
#define TYPEMAP_48(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_46(__VA_ARGS__) >
#define TYPEMAP_50(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_48(__VA_ARGS__) >
#define TYPEMAP_52(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_50(__VA_ARGS__) >
#define TYPEMAP_54(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_52(__VA_ARGS__) >
#define TYPEMAP_56(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_54(__VA_ARGS__) >
#define TYPEMAP_58(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_56(__VA_ARGS__) >
#define TYPEMAP_60(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_58(__VA_ARGS__) >
#define TYPEMAP_62(a,b,...)           mtp::TM::typemap<a,b,TYPEMAP_60(__VA_ARGS__) >
#define TYPEMAP(...)                  XPASTE(TYPEMAP_ ,PP_NARG(__VA_ARGS__)) ( __VA_ARGS__) 
#endif /* TYPEMAP */


#define TYPEMAP_KEY(k)  struct k { \
    static const char * value() \
    { return # k; } \
};

namespace mtp {

    namespace TM {

        struct null 
        {
            static const char * value() { return NULL; };
        };

        template <typename K, typename V, typename N>
        struct typemap 
        {  
            typedef K key; 
            typedef V type;
            typedef N next; 
        };

        // get<key, typemap>::type
        //

        template <typename K, typename M> struct get; 
        template <typename K, typename T, typename N>
        struct get<K, typemap<K, T, N> >
        {
            typedef T type;
        }; 
        template <typename K, typename H, typename T, typename N>
        struct get<K, typemap<H,T,N> >
        {
            typedef typename get<K,N>::type type;
        };

        // append<key, type_value, typemap>::type
        //

        template <typename K, typename V, typename N> struct append;
        template <typename K, typename V>
        struct append<K, V, null>
        {
            typedef typemap<K,V,null> type;
        };

        template <typename K, typename V, typename U,typename N>
        struct append<K, V, typemap<K,U,N> >
        {
            // fail to compile: key is already present
        };
        template <typename K, typename H, typename V, typename U, typename N>
        struct append<K, V, typemap<H,U,N> >
        {
            typedef typemap<H,U, typename append<K,V,N >::type> type;
        };       
        
        // size<typemap>::value
        //

        template <class M> struct size;
        template <>
        struct size<null>
        {
            enum { value = 0 };
        };
        template <typename K, typename V, typename N>
        struct size<typemap<K,V,N> >
        {
            enum { value = 1 + size<N>::value };
        };

        // indexof<key>::value
        //

        template <typename K, typename M> struct indexof;
        template <typename K>
        struct indexof<K, null>
        { 
            enum { value = -1 };
        };
        template <typename K, typename V, typename N>
        struct indexof<K, typemap<K, V, N> >
        {
            enum { value = 0 };
        };
        template <typename K, typename H, typename V, typename N>
        struct indexof<K, typemap<H, V, N> >
        {
            enum { value = indexof<K,N>::value == -1 ? -1 : 1 + indexof<K,N>::value  };
        };
     
        // get_key<int i, typemap>::type
        //

        template <int i, typename M> struct get_key;
        template <int i, typename K, typename V, typename N>
        struct get_key< i, typemap<K,V,N> >
        {
            typedef typename get_key<i-1, N>::type type;
        };
        template <typename K, typename V, typename N>
        struct get_key<0, typemap<K,V,N> >
        {
            typedef K type;
        };
        template <int i>
        struct get_key<i, null>
        {
            typedef null type;
        };
    }
}

#endif /* _TYPEMAP_HH_ */
