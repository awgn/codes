/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli
 * ----------------------------------------------------------------------------
 */

#ifndef SINGLETON_HH
#define SINGLETON_HH

#include <tr1/type_traits>
#include <iostream>
#include <typelist.hh>

namespace more
{    
    struct singleton_type {};
    struct indestructible_singleton_type {};

    template <typename T, typename CV = singleton_type, typename S = more::TL::null, 
              int N = more::TL::length<S>::value,                 /* for partial specialization */
              typename B = typename std::tr1::remove_cv<CV>::type /* for partial specialization */ > class singleton;
    struct singleton_base
    {
        template <typename T>
        struct identity
        {
            typedef T type;
        };

        template <typename U, bool c, bool v> struct __add_cv;
        template <typename U>
        struct __add_cv<U, true, false>
        {
            typedef typename std::tr1::add_const<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, false, true>
        {
            typedef typename std::tr1::add_volatile<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, true, true>
        {
            typedef typename std::tr1::add_cv<U>::type type;
        };
        template <typename U>
        struct __add_cv<U, false, false>
        {
            typedef U type;
        };

        template <typename U, typename V>
        struct add_cv_qualifier
        {
            typedef typename __add_cv<U, std::tr1::is_const<V>::value, std::tr1::is_volatile<V>::value>::type type;
        };

    private:
        singleton_base(const singleton_base&);                // noncopyable
        singleton_base &operator=(const singleton_base &);    // noncopyable  

    protected:
        singleton_base()
        {}
        
        ~singleton_base()
        {}
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,0,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,0, singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance()
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one( (typename base_type::tag()) );
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,1,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,1,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one( typename base_type::tag(), x1);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,2,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,2,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,3,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,3,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2, x3);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,4,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,4,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(),
                                                                                 arg4_type x4 = arg4_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2, x3, x4);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,5,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,5,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2, x3, x4, x5);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,6,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,6,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2, x3, x4, x5, x6);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,7,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;
        typedef typename more::TL::at<S,6>::type arg7_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,7,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type(), 
                                                                                 arg7_type x7 = arg7_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2, x3, x4, x5, x6, x7);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,8,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;
        typedef typename more::TL::at<S,6>::type arg7_type;
        typedef typename more::TL::at<S,7>::type arg8_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,8,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type(), 
                                                                                 arg7_type x7 = arg7_type(), arg8_type x8 = arg8_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2, x3, x4, x5, x6, x7, x8);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,9,singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;
        typedef typename more::TL::at<S,6>::type arg7_type;
        typedef typename more::TL::at<S,7>::type arg8_type;
        typedef typename more::TL::at<S,8>::type arg9_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,9,singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type(), 
                                                                                 arg7_type x7 = arg7_type(), arg8_type x8 = arg8_type(), arg9_type x9 = arg9_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one(typename base_type::tag(), x1, x2, x3, x4, x5, x6, x7, x8, x9);
            return one;
        }
    };

    ///////////////////////////////////////////////// indestructible_singleton_type 

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,0,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,0, indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance()
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type * one = 
                new typename singleton_base::add_cv_qualifier<T,CV>::type( (typename base_type::tag()) );
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,1,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,1,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type * one =
                new typename singleton_base::add_cv_qualifier<T,CV>::type( typename base_type::tag(), x1);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,2,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,2,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type *one =
                new typename singleton_base::add_cv_qualifier<T,CV>::type(typename base_type::tag(), x1, x2);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,3,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,3,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type *one =
                new typename singleton_base::add_cv_qualifier<T,CV>::type(typename base_type::tag(), x1, x2, x3);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,4,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,4,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(),
                                                                                 arg4_type x4 = arg4_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type *one = 
                new typename singleton_base::add_cv_qualifier<T,CV>::type(typename base_type::tag(), x1, x2, x3, x4);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,5,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,5,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type *one =
                new typename singleton_base::add_cv_qualifier<T,CV>::type(typename base_type::tag(), x1, x2, x3, x4, x5);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,6,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,6,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type * one =
                new typename singleton_base::add_cv_qualifier<T,CV>::type(typename base_type::tag(), x1, x2, x3, x4, x5, x6);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,7,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;
        typedef typename more::TL::at<S,6>::type arg7_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,7,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type(), 
                                                                                 arg7_type x7 = arg7_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type *one = 
                new typename singleton_base::add_cv_qualifier<T,CV>::type (typename base_type::tag(), x1, x2, x3, x4, x5, x6, x7);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,8,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;
        typedef typename more::TL::at<S,6>::type arg7_type;
        typedef typename more::TL::at<S,7>::type arg8_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,8,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type(), 
                                                                                 arg7_type x7 = arg7_type(), arg8_type x8 = arg8_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type *one = 
                new typename singleton_base::add_cv_qualifier<T,CV>::type(typename base_type::tag(), x1, x2, x3, x4, x5, x6, x7, x8);
            return *one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,9,indestructible_singleton_type> : public singleton_base
    {
        friend class singleton_base::identity<T>::type;
        typedef typename more::TL::at<S,0>::type arg1_type;
        typedef typename more::TL::at<S,1>::type arg2_type;
        typedef typename more::TL::at<S,2>::type arg3_type;
        typedef typename more::TL::at<S,3>::type arg4_type;
        typedef typename more::TL::at<S,4>::type arg5_type;
        typedef typename more::TL::at<S,5>::type arg6_type;
        typedef typename more::TL::at<S,6>::type arg7_type;
        typedef typename more::TL::at<S,7>::type arg8_type;
        typedef typename more::TL::at<S,8>::type arg9_type;

        struct tag {};

    public:
        typedef singleton<T,CV,S,9,indestructible_singleton_type> base_type;

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1_type x1 = arg1_type(), arg2_type x2 = arg2_type(), arg3_type x3 = arg3_type(), 
                                                                                 arg4_type x4 = arg4_type(), arg5_type x5 = arg5_type(), arg6_type x6 = arg6_type(), 
                                                                                 arg7_type x7 = arg7_type(), arg8_type x8 = arg8_type(), arg9_type x9 = arg9_type() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type *one = 
                new typename singleton_base::add_cv_qualifier<T,CV>::type(typename base_type::tag(), x1, x2, x3, x4, x5, x6, x7, x8, x9);
            return *one;
        }
    };

} // namespace more

#endif /* SINGLETON_HH */
