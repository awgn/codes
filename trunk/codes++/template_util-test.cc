/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#include <template_util.hh>

template <typename T1 = more::null_type>
struct test_1
{
    test_1()
    {
        std::cout << __FUNCTION__ << ": " << more::count_param_1<T1>::value << std::endl;
    }
};


template <typename T1 = more::null_type, typename T2 = more::null_type>
struct test_2
{
    test_2()
    {
        std::cout << __FUNCTION__ << ": " << more::count_param_2<T1,T2>::value << std::endl;
    }
};

template <typename T1 = more::null_type, typename T2 = more::null_type, typename T3 = more::null_type>
struct test_3
{
    test_3()
    {
        std::cout << __FUNCTION__ << ": " << more::count_param_3<T1,T2,T3>::value << std::endl;
    }
};

template <typename T1 = more::null_type, typename T2 = more::null_type, typename T3 = more::null_type,
          typename T4 = more::null_type>
struct test_4
{
    test_4()
    {
        std::cout << __FUNCTION__ << ": " << more::count_param_4<T1,T2,T3,T4>::value << std::endl;
    }
};

template <typename T1 = more::null_type, typename T2 = more::null_type, typename T3 = more::null_type,
          typename T4 = more::null_type, typename T5 = more::null_type>
struct test_5
{
    test_5()
    {
        std::cout << __FUNCTION__ << ": " << more::count_param_5<T1,T2,T3,T4,T5>::value << std::endl;
    }
};

template <typename T1 = more::null_type, typename T2 = more::null_type, typename T3 = more::null_type,
          typename T4 = more::null_type, typename T5 = more::null_type, typename T6 = more::null_type>
struct test_6
{
    test_6()
    {
        std::cout << __FUNCTION__ << ": " << more::count_param_6<T1,T2,T3,T4,T5,T6>::value << std::endl;
    }
};




int
main(int argc, char *argv[])
{
    {
        test_1<> a0;
        test_1<int> a1;
    }

    {
        test_2<> a0;
        test_2<int> a1;
        test_2<int,int> a2;
    }

    {
        test_3<> a0;
        test_3<int> a1;
        test_3<int,int> a2;
        test_3<int,int,int> a3;
    }

    { 
        test_4<> a0;
        test_4<int> a1;
        test_4<int,int> a2;
        test_4<int,int,int> a3;
        test_4<int,int,int,int> a4;
    }

    { 
        test_5<> a0;
        test_5<int> a1;
        test_5<int,int> a2;
        test_5<int,int,int> a3;
        test_5<int,int,int,int> a4;
        test_5<int,int,int,int,int> a5;
    }

    { 
        test_6<> a0;
        test_6<int> a1;
        test_6<int,int> a2;
        test_6<int,int,int> a3;
        test_6<int,int,int,int> a4;
        test_6<int,int,int,int,int> a5;
        test_6<int,int,int,int,int,int> a6;
    }



    return 0;
}
 
