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

    template <typename T, typename CV = singleton_type, typename S = mtp::TL::null, int N = mtp::TL::length<S>::value > class singleton;

    struct singleton_base
    {
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
    class singleton<T,CV,S,0> : public singleton_base
    {
    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,0>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag &)
        {} 

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance()
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()));
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,1> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,1>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,2> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;
        typedef typename mtp::TL::at<S,1>::type arg2;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,2>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1(), arg2 x2 = arg2() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1, x2);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,3> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;
        typedef typename mtp::TL::at<S,1>::type arg2;
        typedef typename mtp::TL::at<S,2>::type arg3;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,3>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1(), arg2 x2 = arg2(), arg3 x3 = arg3() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1, x2, x3);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,4> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;
        typedef typename mtp::TL::at<S,1>::type arg2;
        typedef typename mtp::TL::at<S,2>::type arg3;
        typedef typename mtp::TL::at<S,3>::type arg4;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,4>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1(), arg2 x2 = arg2(), arg3 x3 = arg3(), arg4 x4 = arg4() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1, x2, x3, x4);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,5> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;
        typedef typename mtp::TL::at<S,1>::type arg2;
        typedef typename mtp::TL::at<S,2>::type arg3;
        typedef typename mtp::TL::at<S,3>::type arg4;
        typedef typename mtp::TL::at<S,4>::type arg5;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,5>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1(), arg2 x2 = arg2(), arg3 x3 = arg3(), arg4 x4 = arg4(), 
                                                                                 arg5 x5 = arg5() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1, x2, x3, x4, x5);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,6> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;
        typedef typename mtp::TL::at<S,1>::type arg2;
        typedef typename mtp::TL::at<S,2>::type arg3;
        typedef typename mtp::TL::at<S,3>::type arg4;
        typedef typename mtp::TL::at<S,4>::type arg5;
        typedef typename mtp::TL::at<S,5>::type arg6;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,6>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1(), arg2 x2 = arg2(), arg3 x3 = arg3(), arg4 x4 = arg4(), 
                                                                                 arg5 x5 = arg5(), arg6 x6 = arg6() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1, x2, x3, x4, x5, x6);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,7> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;
        typedef typename mtp::TL::at<S,1>::type arg2;
        typedef typename mtp::TL::at<S,2>::type arg3;
        typedef typename mtp::TL::at<S,3>::type arg4;
        typedef typename mtp::TL::at<S,4>::type arg5;
        typedef typename mtp::TL::at<S,5>::type arg6;
        typedef typename mtp::TL::at<S,6>::type arg7;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,7>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1(), arg2 x2 = arg2(), arg3 x3 = arg3(), arg4 x4 = arg4(), 
                                                                                 arg5 x5 = arg5(), arg6 x6 = arg6(), arg7 x7 = arg7() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1, x2, x3, x4, x5, x6, x7);
            return one;
        }
    };

    template <typename T,typename CV, typename S>
    class singleton<T,CV,S,8> : public singleton_base
    {
        typedef typename mtp::TL::at<S,0>::type arg1;
        typedef typename mtp::TL::at<S,1>::type arg2;
        typedef typename mtp::TL::at<S,2>::type arg3;
        typedef typename mtp::TL::at<S,3>::type arg4;
        typedef typename mtp::TL::at<S,4>::type arg5;
        typedef typename mtp::TL::at<S,5>::type arg6;
        typedef typename mtp::TL::at<S,6>::type arg7;
        typedef typename mtp::TL::at<S,7>::type arg8;

    protected:
        struct singleton_tag {
            friend class singleton<T,CV,S,8>;
        private:
            singleton_tag()  {}
        };

    public:
        singleton(singleton_tag)
        {}

        // singleton instance...
        //
        static typename singleton_base::add_cv_qualifier<T,CV>::type & instance( arg1 x1 = arg1(), arg2 x2 = arg2(), arg3 x3 = arg3(), arg4 x4 = arg4(), 
                                                                                 arg5 x5 = arg5(), arg6 x6 = arg6(), arg7 x7 = arg7(), arg8 x8 = arg8() )
        {
            static typename singleton_base::add_cv_qualifier<T,CV>::type one((singleton_tag()), x1, x2, x3, x4, x5, x6, x7, x8);
            return one;
        }
    };

} // namespace more

#endif /* SINGLETON_HH */
